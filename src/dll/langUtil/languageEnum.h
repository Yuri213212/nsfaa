#include <string.h>

#define languageCount	4
enum languageEnum{
	LANG_dummy=0,
	LANG_EN,
	LANG_CH,
	LANG_JP,
};

const char language[languageCount][3]={
	"..",
	"en",
	"ch",
	"jp",
};

int getLanguageEnum(char *s){
	int i;

	if (!s) return LANG_dummy;
	for (i=0;i<languageCount;++i){
		if (!strcmp(s,language[i])) return i;
	}
	return LANG_dummy;
}
