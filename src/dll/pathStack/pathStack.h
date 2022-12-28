#include <stdio.h>
#include <stdlib.h>
#include "../vlstr.h"

#define PATH_PDMARK	'?'

typedef void (*_DELETE)(void *);

struct pathStack{
	struct pathStack *prev;
	struct pathStack *next;
	HVLSTR filePath;	//<dir>/<filename>	//path of ? (program directory) in root
	HVLSTR dir;		//<dir>			//path of ., current directory on init for root
	HVLSTR relPath;
	FILE *fp;
	void *data;
	_DELETE _deleteData;
};
