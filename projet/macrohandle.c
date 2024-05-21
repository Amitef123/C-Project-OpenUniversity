#include "macrohandle.h"
#define MACRO_NAME 80
#define MACRO_LENGTH 82*6

char line[MACRO_NAME+2];/*max size of line*/
/*macro list struct*/
typedef struct mcnode {
	char *mcname;
	char *mccontent;
	struct mcnode *next;
}macroNode;

macroNode *macroList;
/*counter for macro*/
int macro_counter=0;

/*
	process a macro function on the file and make a new file
	@param file out put name and open file
	@return void
*/
void macrohandle(char *filename,FILE *fd)
{

	FILE *mc;/*output file*/
	mc=fopen(filename,"w+");
	while (fgets(line,82,fd)!=NULL)
	{
		if(!is_macro(fd,mc,line)&&!is_new_macro(fd,line))
		{
			fputs(line,mc);
		}
		
	}
	fclose(mc);
	
}
/*
	check if the line is a place to set macro
	@param output file input file and line
	@reutrn bool if it is a macro or not
*/
int is_macro(FILE *fd,FILE *mc, char *line)
{
	int i=macro_counter;

	macroNode *macroSearch= macroList;
	for(;i>0;i--)
	{
		if(!compare_string(line,macroSearch->mcname))
		{
			i=-1;
			break;
		}
		else
			macroSearch=macroSearch->next;
	}
	if(i==-1)
	{
		fputs(macroSearch->mccontent,mc);
		return 1;
	}
	return 0;
}
/*
	check if this is a set for new macro and if it is insert to the list
	@param input file and line
	@return bool if it is a set for a new macro
*/
int is_new_macro (FILE *fd,char *line)
{
	int i;
	char *tempmcname;
	char *tempmccontent;
	line=skip_spaces(line);	
	if(strncmp(line,"macro",5)==0)
	{
		line=line+5;
		line=skip_spaces(line);
		tempmccontent=(char*)malloc(sizeof(char)*MACRO_LENGTH);
		tempmcname=(char*)malloc(sizeof(char)*MACRO_NAME);
		for(i=0;line[i]!=0&&(!isspace(line[i]));i++)
		{
			tempmcname[i]=line[i];
		}
		fgets(line,82,fd);
		for(i=0;(compare_string(line,"endm"))&&!feof(fd)&&i<6;i++)
		{
			strcat(tempmccontent,line);
			fgets(line,82,fd);
		}
		if(macro_counter==0)
		{
			macroList=(macroNode*)malloc(sizeof(macroNode));
			macroList->mcname=tempmcname;
			macroList->mccontent=tempmccontent;
			macroList->next=NULL;
			macro_counter++;
		}
		else
		{
			macroNode *tempNode=(macroNode*)malloc(sizeof(macroNode));
			tempNode->mcname=tempmcname;
			tempNode->mccontent=tempmccontent;
			tempNode->next=macroList;
			macroList=tempNode;
			macro_counter++;
		}
	return 1;
	}
	return 0;	
}
/* 
	skip spaces
	@parm a pointer for string
	@return a pointer for place without spaces
	*/
char *skip_spaces(char *s)
 {
    int n;
    for (n = 0; s[n] != 0 && (s[n] == ' ' || s[n] == '\t'); n++) {} /* Count how many spaces there are at the beginning */
    return s + n;
 }
 /*
 	copmare between strings without spaces
 	@param two string
 	@return bool if they are equal
 */
 int compare_string(char *str1, char *str2)
{	
	str1=skip_spaces(str1);
	str2=skip_spaces(str2);
	
	for(;(*str1)==(*str2);str1++,str2++)
	{
		if(*str1=='\0')
			return 0;
	}
	if(*str2=='\0')
	{
		for(;*str1==' '||*str1=='\t'||*str1=='\n';str1++){}
		if(*str1=='\0')
			return 0;
	}
	return 1;
}
		


