/* Contains major function that are related to the first pass */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "settings.h"
#include "code.h"
#include "utils.h"
#include "instructions.h"
#include "first_pass.h"

/**
	process_code if there is a code word in the line it insert it into the table
	@param line the line text
	@param i the place that the word code begin
	@param *ic the ic next line
	@param **code_img the ic array
	@return if the proces work currect
*/
static bool process_code(line_info line, int i, long *ic, machine_word **code_img);
/**
	extra_code word check if there any need for another data word to save the operand
	@param **code img for save the data if it is number
	@param ic to make space for the data that we insert or will be insert in the arry
	@param operand so we could check wich data should be inseret
	@return the status if it work ok
*/
static void build_extra_codeword_fpass(machine_word **code_img, long *ic, char *operand);
/* deatils in the first_pass.h*/
bool process_line_fpass(line_info line, long *IC, long *DC, machine_word **code_img, long *data_img,
                        table *symbol_table) {
	int i, j;
	char symbol[MAX_LINE_LENGTH];
	instruction instruction;
	i=0;
	

	MOVE_TO_NOT_WHITE(line.content, i) /* Move to next non-white char */
	if (!line.content[i] || line.content[i] == '\n' || line.content[i] == EOF || line.content[i] == ';')
		return TRUE; /* empty line or comment line*/

/* Check if symbol  line 1.3-1.5 */
	/* if there is valid label save it in symbol if there is no symbol continue if there is a sign for a symbol but the symbol is invalid return false */
	if (find_label(line, symbol)) {
		return FALSE;
	}
	
	/* if there is a symbol but is name is not valid return false */
	if (symbol[0] && !is_valid_label_name(symbol)) {
		printf_line_error(line, "Illegal label name: %s", symbol);
		return FALSE;
	}
	/* if there is a symbol skip to the end of the symbol word */
	if (symbol[0] != '\0') {
		for (; line.content[i] != ':'; i++);
		i++;
	}

	MOVE_TO_NOT_WHITE(line.content, i)

	if (line.content[i] == '\n') return TRUE; /*there is only label in the line*/

	/* check if it alredy defined */
	if (find_by_types(*symbol_table, symbol, 3, EXTERNAL_SYMBOL, DATA_SYMBOL, CODE_SYMBOL)) {
		printf_line_error(line, "Symbol %s is already defined.", symbol);
		return FALSE;
	}
	
	/* Check if it's an instruction (starting with '.') */
	instruction = find_instruction_from_index(line, &i);
	
	if (instruction == ERROR_INST) { /* Syntax error found */
		return FALSE;
	}	
	MOVE_TO_NOT_WHITE(line.content, i)
	
	if (instruction != NONE_INST) {
		/* if .string or .data, and symbol defined, put it into the symbol table line 1.5*/
		if ((instruction == DATA_INST || instruction == STRING_INST) && symbol[0] != '\0'){
			/* is data or string, add DC with the symbol to the table as data */
			add_table_item(symbol_table, symbol, *DC, DATA_SYMBOL);
			}
		/* if string, encode into data image buffer and increase dc as needed. */
		if (instruction == STRING_INST)
			return process_string_instruction(line, i, data_img, DC);
			/* if .data, do same but parse numbers. */
		else if (instruction == DATA_INST)
			return process_data_instruction(line, i, data_img, DC);
			/* if .extern, add to externals symbol table */
		else if (instruction == EXTERN_INST) {
			MOVE_TO_NOT_WHITE(line.content, i)
			/* if external symbol detected, start analyzing from it's deceleration end */
			for (j = 0; line.content[i] && line.content[i] != '\n' && line.content[i] != '\t' && line.content[i] != ' ' && line.content[i] != EOF; i++, j++) {
				symbol[j] = line.content[i];
			}
			symbol[j] = '\0';
			/* If invalid external label name, it's an error */
			if (!is_valid_label_name(symbol)) {
				printf_line_error(line, "Invalid external label name: %s", symbol);
				return TRUE;
			}
			add_table_item(symbol_table, symbol, 0, EXTERNAL_SYMBOL); /* Extern value is defaulted to 0 */
		}
		/* if entry and symbol defined, print error */
		else if (instruction == ENTRY_INST && symbol[0] != '\0') {
			printf_line_error(line, "Can't define a label to an entry instruction.");
			return FALSE;
		}
		/* .entry is handled in second pass! */
	} 
	else {
	
		/* if there is a code and no instruction add symbol to table */
		if (symbol[0] != '\0')
			add_table_item(symbol_table, symbol, *IC, CODE_SYMBOL);
		/* Analyze code */
		return process_code(line, i, IC, code_img);
	}
	return TRUE;
}


static bool process_code(line_info line, int i, long *ic, machine_word **code_img) {
	char operation[8]; /* stores the string of the current code instruction */
	char *operands[2]; /* 2 strings, each for operand */
	opcode curr_opcode; /* the current opcode and funct values */
	funct curr_funct;
	code_word *codeword; /* The current code word */
	opcode_word *opword;/* The current opword */
	long ic_before;
	int j, operand_count;
	machine_word *word_to_write;

	MOVE_TO_NOT_WHITE(line.content, i)

	/* Until white char, end of line, or too big function copy it: */
	for (j = 0; line.content[i] && line.content[i] != '\t' && line.content[i] != ' ' && line.content[i] != '\n' && line.content[i] != EOF && j < 6; i++, j++) {
		operation[j] = line.content[i];
	}
	operation[j] = '\0'; /* End of string */
	/* Get opcode & funct by command name into curr_opcode/curr_funct */
	get_opcode_func(operation, &curr_opcode, &curr_funct);
	/* If invalid operation opcode is NONE_OP=-1. */
	if (curr_opcode == NONE_OP) {
		printf_line_error(line, "Unrecognized instruction: %s.", operation);
		return FALSE; /* an error occurred */
	}
	
		/* Separate operands save each one in arry of oprands and get their count */
	if (!analyze_operands(line, i, operands, &operand_count, operation))  {
		return FALSE;
	}
	
	/* Build opword struct to store the op word array */
	if ((opword = build_opword(line, curr_opcode)) == NULL) {
		return FALSE;
		}
	
	/* ic in position of new code word */
	ic_before = *ic;

	/* allocate memory for a new opword in the code image, and put the opword into it */
	word_to_write = (machine_word *) malloc_with_check(sizeof(machine_word));
	(word_to_write->word).opword = opword;
	code_img[(*ic) -IC_INIT_VALUE] = word_to_write; 
	/* if there are more than one oprand we need to make a code word */
	if (operand_count>0) { /* If there's 1 operand at least */
		if ((codeword = get_code_word(line, curr_opcode, curr_funct, operand_count, operands)) == NULL) {
		/* Release allocated memory for operands if there was a problem with create the code word */
			if (operands[0]) {
				free(operands[0]);
				if (operands[1]) {
					free(operands[1]);
				}
			}
			return FALSE;
			}
			/* ic ++ because we add an code word*/
		(*ic)++;
		/*create space for the code word and save it in the img arry*/
		word_to_write = (machine_word *) malloc_with_check(sizeof(machine_word));
		(word_to_write->word).code = codeword;
		code_img[(*ic) -IC_INIT_VALUE] = word_to_write; 
	         		code_img[(*ic) - IC_INIT_VALUE]->length = -1;/*set the code word length to -1 so we can differ between data word and opword*/
	         	}

/* Build extra information code word if possible, free pointers with no need */
	if (operand_count--) { /* If there's 1 operand at least */
		build_extra_codeword_fpass(code_img, ic, operands[0]);
		free(operands[0]);
		if (operand_count) { /* If there are 2 operands */
			build_extra_codeword_fpass(code_img, ic, operands[1]);
			free(operands[1]);
		}
	}


	(*ic)++; /* increase ic to point the next free cell cell */

	/* Add the final length (of op word+ code word + data words) to the code word struct: */
	code_img[ic_before - IC_INIT_VALUE]->length = (*ic) - ic_before;

	return TRUE; /* No errors */
}

static void build_extra_codeword_fpass(machine_word **code_img, long *ic, char *operand) {
	addressing_type operand_addressing = get_addressing_type(operand);
	/*if another data word is required, increase IC for place to offset and base. if it's an immediate addressing, encode it. */
	if (operand_addressing != NONE_ADDR && operand_addressing != REGISTER_ADDR) {
		if (operand_addressing == IMMEDIATE_ADDR) {
			char *ptr;
			machine_word *word_to_write;
			/* skip the # in the call to strtol */
			long value = strtol(operand + 1, &ptr, 10);
			(*ic)++;
			word_to_write = (machine_word *) malloc_with_check(sizeof(machine_word));
			word_to_write->length = 0; /* Not Code word or op word */
			(word_to_write->word).data = build_data_word(IMMEDIATE_ADDR, value, FALSE);
			code_img[(*ic) - IC_INIT_VALUE] = word_to_write;
		}
		else
		{
			(*ic)++;
			(*ic)++;
		}
	}
}

