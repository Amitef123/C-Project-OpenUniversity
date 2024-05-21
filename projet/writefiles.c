#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "utils.h"
#include "table.h"

#define KEEP_ONLY_E_LSB(value) ((value) & (0xF))

#define KEEP_ONLY_D_LSB(value) (((value) & (0xF0))>>4)

#define KEEP_ONLY_C_LSB(value) (((value) & (0xF00))>>8)

#define KEEP_ONLY_B_LSB(value) (((value) & (0xF000))>>12)

#define KEEP_ONLY_A_LSB(value) ((value) & ((0xF0000)))

int printBin(int x);
/*process and print opword*/
void build_word_pre_print_opword(machine_word *code_img,FILE *file_desc)
{
	unsigned int x = 0;
	int a;
	int b;
	int c;
	int d;
	int e;

	x=(code_img->word.opword->ARE<<16)|(code_img->word.opword->opcode);
	e=(x&0XF);
	x=x>>4;
	d=(x&0Xf);
	x=x>>4;
	c=(x&0Xf);
	x=x>>4;
	b=(x&0Xf);
	x=x>>4;
	a=(x&0Xf);
	fprintf(file_desc, "A%x",a);
	fprintf(file_desc, " B%x",b);
	fprintf(file_desc, " C%x",c);
	fprintf(file_desc, " D%x",d);
	fprintf(file_desc, " E%x",e);

}
/*process and print code word*/
void build_word_pre_print_codeword(machine_word *code_img,FILE *file_desc)
{
	unsigned int x = 0;
	int a;
	int b;
	int c;
	int d;
	int e;

	x=(code_img->word.code->ARE<<16)|(code_img->word.code->funct<<12)|(code_img->word.code->src_register<<8)|
	(code_img->word.code->src_addressing<<6)|(code_img->word.code->dest_register<<2)|
	(code_img->word.code->dest_addressing);
	e=(x&0XF);
	x=x>>4;
	d=(x&0Xf);
	x=x>>4;
	c=(x&0Xf);
	x=x>>4;
	b=(x&0Xf);
	x=x>>4;
	a=(x&0Xf);
	fprintf(file_desc, "A%x",a);
	fprintf(file_desc, " B%x",b);
	fprintf(file_desc, " C%x",c);
	fprintf(file_desc, " D%x",d);
	fprintf(file_desc, " E%x",e);

}
/*process and print data word*/
void build_word_pre_print_data(machine_word *code_img,FILE *file_desc)
{
	unsigned int x = 0;
	int a;
	int b;
	int c;
	int d;
	int e;

	x=(code_img->word.data->ARE<<16)|(code_img->word.data->data);
	e=(x&0XF);
	x=x>>4;
	d=(x&0Xf);
	x=x>>4;
	c=(x&0Xf);
	x=x>>4;
	b=(x&0Xf);
	x=x>>4;
	a=(x&0Xf);
	fprintf(file_desc, "A%x",a);
	fprintf(file_desc, " B%x",b);
	fprintf(file_desc, " C%x",c);
	fprintf(file_desc, " D%x",d);
	fprintf(file_desc, " E%x",e);

}
void build_word_pre_print_dataimg(unsigned int y,FILE *file_desc)
{
	int x = 0;
	int a;
	int b;
	int c;
	int d;
	int e;
	x=(4<<16)|(y&0xffff);
	e=(x&0XF);
	x=x>>4;
	d=(x&0Xf);
	x=x>>4;
	c=(x&0Xf);
	x=x>>4;
	b=(x&0Xf);
	x=x>>4;
	a=(x&0Xf);
	fprintf(file_desc, "A%x",a);
	fprintf(file_desc, " B%x",b);
	fprintf(file_desc, " C%x",c);
	fprintf(file_desc, " D%x",d);
	fprintf(file_desc, " E%x",e);

}

int printBin(int x)
{
	unsigned int mask=1<<(sizeof(int)*8-1);
	printf( "\n");
	while (mask)
	{
		if(x&mask)
			printf( "1");
		else
			printf( "0");
		mask=mask>>1;
	}
	printf( "\t");
return 0;	
}

static bool write_ob(machine_word **code_img, long *data_img, long icf, long dcf, char *filename);

static bool write_table_to_file_ext(table tab, char *filename) {
	FILE *file_desc;
	/* concatenate filename & extension, and open the file for writing: */
	char *full_filename = strallocat(filename, ".ext");
	file_desc = fopen(full_filename, "w");
	free(full_filename);
	/* if failed, print error and exit */
	if (file_desc == NULL) {
		printf("Can't create or rewrite to file %s.", full_filename);
		return FALSE;
	}
	/* if table is null, nothing to write */
	if (tab == NULL) {
	fprintf(file_desc, "is null");
	return TRUE;
	}
	/* Write first line without \n to avoid extraneous line breaks */
	fprintf(file_desc, "%s BASE %.4ld\n", tab->key, tab->value);
	fprintf(file_desc, "%s OFFSEET %.4ld", tab->key, (tab->value)+1);
	while ((tab = tab->next) != NULL) {
		fprintf(file_desc, "\n\n%s BASE %.4ld\n", tab->key, tab->value);
		fprintf(file_desc, "%s OFFSET %.4ld", tab->key, (tab->value)+1);
	}
	fclose(file_desc);
	return TRUE;
}

static bool write_table_to_file_ent(table tab, char *filename) {
	FILE *file_desc;
	/* concatenate filename & extension, and open the file for writing: */
	char *full_filename = strallocat(filename, ".ent");
	file_desc = fopen(full_filename, "w");
	free(full_filename);
	/* if failed, print error and exit */
	if (file_desc == NULL) {
		printf("Can't create or rewrite to file %s.", full_filename);
		return FALSE;
	}
	/* if table is null, nothing to write */
	if (tab == NULL) return TRUE;

	/* Write first line */
	fprintf(file_desc, "%s,%d,%d", tab->key, tab->base,tab->offset);
	while ((tab = tab->next) != NULL) {
		fprintf(file_desc, "\n%s,%d,%d", tab->key, tab->base,tab->offset);
	}
	fclose(file_desc);
	return TRUE;
}


int write_output_files(machine_word **code_img, long *data_img, long icf, long dcf, char *filename,
                       table symbol_table) {
	bool result;
	
	table externals = filter_table_by_type(symbol_table, EXTERNAL_REFERENCE);
	table entries = filter_table_by_type(symbol_table, ENTRY_SYMBOL);
	/* Write .ob file */
	result = write_ob(code_img, data_img, icf, dcf, filename) &&
	         /* Write *.ent and *.ext files: call with symbols from external references type or entry type only */
	         write_table_to_file_ext(externals, filename) &&
	         write_table_to_file_ent(entries, filename);
	/* Release filtered tables */
	
	free_table(externals);
	free_table(entries);
	return result;

}

static bool write_ob(machine_word **code_img, long *data_img, long icf, long dcf, char *filename) {
	int i;
	FILE *file_desc;
	/* add extension of file to open */
	char *output_filename = strallocat(filename, ".ob");
	/* Try to open the file for writing */
	file_desc = fopen(output_filename, "w");
	free(output_filename);
	if (file_desc == NULL) {
		printf("Can't create or rewrite to file %s.", output_filename);
		return FALSE;
	}

	/* print data/code word count on top */
	fprintf(file_desc, "%ld %ld", icf - IC_INIT_VALUE, dcf);
	for(i=0; i < icf - IC_INIT_VALUE;i++){
		fprintf(file_desc, "\n%.4d\t" , 100+i);
		if(code_img[i]->length ==-1){
			build_word_pre_print_codeword(code_img[i],file_desc);
		
		}
		else if(code_img[i]->length >=2){
			build_word_pre_print_opword(code_img[i],file_desc);
		
			}
		else{
			build_word_pre_print_data(code_img[i],file_desc);
	
		}
	}
	
	for (i = 0; i < dcf; i++) {
	
		fprintf(file_desc, "\n%.4ld\t" , icf+i);
		build_word_pre_print_dataimg(data_img[i],file_desc);
	}
	/* Close the file */
	fclose(file_desc);
	return TRUE;
}

	
