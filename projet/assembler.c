#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "writefiles.h"
#include "utils.h"
#include "first_pass.h"
#include "second_pass.h"
#include "macrohandle.h"

/*
 	Processes an assembly source file, and returns the result status.
	@param filename The filename,
 	@return Whether succeeded
 */
static bool process_file(char *filename);


int main(int argc, char *argv[]) {
	int i;
	bool succeeded = TRUE;
	char *output_filename;
	char *input_filename;
	FILE *file_des;
	/* run untill there is no more file */
	for (i = 1; i < argc; ++i) {
		/* make a signal if one file have an error */
		if (!succeeded) puts("");
		/* add extinsion for the file name*/
		output_filename = strallocat(argv[i],".am");
		input_filename = strallocat(argv[i],".as");
		file_des = fopen(input_filename, "r");
		if (file_des == NULL) {
			printf("Error: file \"%s.as\" is inaccessible for reading. skipping it.\n", argv[i]);
			free(output_filename); 
			succeeded= FALSE;
		}
		/*send the file for macro handle, there is no error in the macro file*/
		macrohandle(output_filename,file_des);
	
		succeeded = process_file(argv[i]);
		
	}
	return 0;
}


static bool process_file(char *filename) {
	int temp_c;/*for finish the line if it exagurate the limt*/
	long ic = IC_INIT_VALUE, dc = 0, icf, dcf; /* line 1.1 init value*/
	bool status = TRUE;
	char *input_filename;
	char temp_line[MAX_LINE_LENGTH + 2]; /* temporary string for each line*/
	FILE *file_des; /*file descriptor to process */
	long data_img[CODE_ARR_IMG_LENGTH]; /* Contains an image of the machine code */
	machine_word *code_img[CODE_ARR_IMG_LENGTH];
	/* Our symbol table */
	table symbol_table = NULL;
	line_info curr_line_info;
	/* Concat extensionless filename with .am extension */
	input_filename = strallocat(filename, ".am");
	/* Open file, skip on failure */
	file_des = fopen(input_filename, "r");
	if (file_des == NULL) {
		/* if file couldn't be opened, write to stderr. */
		printf("Error: file \"%s.am\" is inaccessible for reading. skipping it.\n", filename);
		free(input_filename); /* The only allocated space is for the full file name */
		return FALSE;
	}

	/*first pass: */
	curr_line_info.file_name = input_filename;
	curr_line_info.content = temp_line; /* copy each line for temp line. */
	for (curr_line_info.line_number = 1;
	     fgets(temp_line, MAX_LINE_LENGTH + 2, file_des) != NULL; curr_line_info.line_number++) /*line 1.2 read the next line*/
	     {    

		/* check if line is to exagurate. */
		if (strchr(temp_line, '\n') == NULL && !feof(file_des)) {
			/* print error message  */
			printf_line_error(curr_line_info, "Line is to big, the Maximum length is- %d.",
			                  MAX_LINE_LENGTH);              
			status= FALSE;/*change status*/
			/* skip to the line */
			do {
				temp_c = fgetc(file_des);
			} while (temp_c != '\n' && temp_c != EOF);/*finish the line running*/
		} else {
			/*call for the process line in fpass*/
			if (!process_line_fpass(curr_line_info, &ic, &dc, code_img, data_img, &symbol_table)) {
				if (status) {
					/*free_code_image(code_img, ic_before);*/
					icf = -1;
					status = FALSE;
				}
			}
		}
	}
	/*finish first pass*/
	/* Save ICF & DCF (1.18) */
	icf = ic;
	dcf = dc;

	/* check status for start second pass */
	if (status) {

	ic = IC_INIT_VALUE;

	/* update the value in each symbol in the table (step 1.19) */
	add_value_to_type(symbol_table, icf, DATA_SYMBOL);

	/*start second pass: */
	rewind(file_des); /* back to the start of the file */
	for (curr_line_info.line_number = 1; !feof(file_des); curr_line_info.line_number++) {
		int i = 0;
		fgets(temp_line, MAX_LINE_LENGTH, file_des); /* Get line */
		MOVE_TO_NOT_WHITE(temp_line, i)
		if ((code_img[ic - IC_INIT_VALUE] != NULL)|| temp_line[i] == '.')/*check if the op ic of the line is not null and if 				the line is instruction*/
		/*call for the main function of the second pass*/
			status =status & process_line_spass(curr_line_info, &ic, code_img, &symbol_table);
	}
		/*finish second pass*/

		/* check for status */
		if (status) {
			/* call for write out put file Write to *filename.ob/.ext/.ent */
			status =status & write_output_files(code_img, data_img, icf, dcf, filename, symbol_table);
		}
	}
	
	/* finish program free pointer */
	
	/* current file name */
	free(input_filename);
	/* Free symbol table */
	free_table(symbol_table);
	/* Free code & data buffer contents */
	free_code_image(code_img, icf);

	/* return the status*/
	return status;
}



