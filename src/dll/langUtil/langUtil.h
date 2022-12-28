#define WINVER		0x0400

#include <windows.h>
#include "../vlstr.h"
#include "../pathStack.h"
#include "../utf8Format.h"
#include "languageEnum.h"

#define LANGID_MASK	0x3FF
#define LANGID_EN	0x009
#define LANGID_CH	0x004
#define LANGID_JP	0x011

const char szLanguageFile[]=		"?/language.txt";
