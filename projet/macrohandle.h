#include <stdio.h> 
#include <stdlib.h>
#include <ctype.h>
 #include <string.h> 

/*
	process a macro function on the file and make a new file
	@param file out put name and open file
	@return void
*/
void macrohandle(char *filename,FILE *fd);
int is_macro(FILE *fd,FILE *mc, char *line);
int is_new_macro (FILE *fd,char *line);
char *skip_spaces(char *s);
 int compare_string(char *str1, char *str2);
