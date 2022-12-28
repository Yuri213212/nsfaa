#ifndef LANGUTIL_H
#define LANGUTIL_H

#include "pathStack.h"

enum languageEnum{
	LANG_dummy=0,
	LANG_EN,
	LANG_CH,
	LANG_JP,
};

//get default UI language
int lang_getUserLang();

//get language by setting
int lang_getLangSetting(HPS hps);

#endif
