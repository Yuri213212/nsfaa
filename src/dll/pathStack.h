#ifndef PATHSTACK_H
#define PATHSTACK_H

#include <stdio.h>
#include <wchar.h>
#include "vlstr.h"

typedef void* HPS;
typedef void (*_DELETE)(void *);

//create path stack object
//root path (?) is where the exe exists, and 1st path (.) is current directory
//call ps_delete to free memory
HPS ps_new();

//destroy path stack object
void ps_delete(HPS hps);

//reset path stack to initial state, close all opening files
void ps_clear(HPS hps);

//push and open file using relative path (auto free)
//start with '.' means relative to last file path
//start with '?' means relative to program path
//return -1 for fail, 1 if the file was in the stack, 0 if the file was not in the stack
int ps_push(HPS hps,HVLSTR filePath,wchar_t *mode);

//pop file and close
void ps_pop(HPS hps);

//get fp of lastest file
FILE *ps_getFp(HPS hps);

//attatch data to lastest file (auto free), free on fail
void ps_attatchData(HPS hps,void *data,_DELETE _deleteData);

//get attatched data of lastest file
void *ps_getData(HPS hps);

//check whether the destructor of attatched data of lastest file matchs
//use the destructor to identify the type of attatched data
int ps_checkDataDestructor(HPS hps,_DELETE _deleteData);

//change current directory to the path of current file
void ps_cd(HPS hps);

//get full path string of current file (auto free)
HVLSTR ps_getFullPath(HPS hps);

//get relative path string of current file (auto free)
HVLSTR ps_getRelPath(HPS hps);

//copy full path string of current file
//call vlstr_delete to free memory
HVLSTR ps_copyFullPath(HPS hps);

//copy relative path string of current file
//call vlstr_delete to free memory
HVLSTR ps_copyRelPath(HPS hps);

//convert absolute file path string to relative path string for output
void ps_convertPath(HVLSTR filePath,HPS hps);

#endif
