#ifndef VLSTR_H
#define VLSTR_H

#include <windows.h>
#include <stdio.h>
#include <wchar.h>

typedef void* HVLSTR;

//create string object
//call vlstr_delete to free memory
HVLSTR vlstr_new();
HVLSTR vlstr_newEx(const char *s);
HVLSTR vlstr_newExw(const wchar_t *s);

//destroy string object
void vlstr_delete(HVLSTR hvlstr);

//clone string object
//call vlstr_delete to free memory
HVLSTR vlstr_clone(HVLSTR hvlstr);

//partially clone string object, length can be minus for end of the string
//call vlstr_delete to free memory
HVLSTR vlstr_subString(HVLSTR hvlstr,int start,int length);

//clear data of string object
void vlstr_clear(HVLSTR hvlstr);

//add char to end
void vlstr_addc(HVLSTR hvlstr,char c);

//add string to end
void vlstr_adds(HVLSTR hvlstr,const char *s);
void vlstr_addsw(HVLSTR hvlstr,const wchar_t *s);

//add result of sprintf to end
void vlstr_addf(HVLSTR hvlstr,const char *format,...);
void vlstr_addfw(HVLSTR hvlstr,const wchar_t *format,...);

//add data of string object to end
void vlstr_addstr(HVLSTR hvlstr,HVLSTR hvlstr2);

//concatenate 2 string object and create new string object
//call vlstr_delete to free memory
HVLSTR vlstr_concat(HVLSTR hvlstr1,HVLSTR hvlstr2);

//get last char of string object
char vlstr_last(HVLSTR hvlstr);

//drop last char from string object
char vlstr_drop(HVLSTR hvlstr);

//read file and create string object, to next blank character(auto trim start)
//call vlstr_delete to free memory
HVLSTR vlstr_read(FILE *fp);

//read file and create string object, to line end(kills new line)
//call vlstr_delete to free memory
HVLSTR vlstr_readLine(FILE *fp);

//read file and create string object, fixed size block in binary
//call vlstr_delete to free memory
HVLSTR vlstr_readBlock(FILE *fp,int length);

//write string to text file
void vlstr_write(FILE *fp,HVLSTR str);

//write string to text file with new line
void vlstr_writeLine(FILE *fp,HVLSTR str);

//write string to binary file in fixed size block
void vlstr_writeBlock(FILE *fp,HVLSTR str,int length);

//get length of string object
int vlstr_length(HVLSTR hvlstr);

//get the pointer to internal data of string object
//do NOT modify the data, the structure may get corrupted
char *vlstr_getData(HVLSTR hvlstr);

//convert current data to UTF16 string (auto free)
wchar_t *vlstr_getDataw(HVLSTR hvlstr);

//get a copy of current data
//call free to free memory
char *vlstr_copyData(HVLSTR hvlstr);
wchar_t *vlstr_copyDataw(HVLSTR hvlstr);

//compare to another string object
int vlstr_compare(HVLSTR hvlstr,HVLSTR hvlstr2);

//remove blank characters from end
void vlstr_trimEnd(HVLSTR hvlstr);

//search and replace char
void vlstr_replaceChar(HVLSTR hvlstr,char search,char to);

//search first position of character, return -1 when failed
int vlstr_indexOf(HVLSTR hvlstr,char c);

//search last position of character, return -1 when failed
int vlstr_lastIndexOf(HVLSTR hvlstr,char c);

//win32api get string functions
//call vlstr_delete to free memory
HVLSTR vlstr_GetCurrentDirectory();
HVLSTR vlstr_GetModuleFileName(HMODULE hModule);
HVLSTR vlstr_DragQueryFile(HDROP hDrop,int iFile);

void vlstr_GetFullPathName(HVLSTR fileName);

#endif
