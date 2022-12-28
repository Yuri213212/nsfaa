#include "langUtil.h"

int lang_getUserLang(){
	LANGID id;

	id=GetUserDefaultLCID();
	switch (id&LANGID_MASK){
	case LANGID_CH:
		return LANG_CH;
	case LANGID_JP:
		return LANG_JP;
	case LANGID_EN:
	default:
		return LANG_EN;
	}
}

int lang_getLangSetting(HPS hps){
	int result;
	FILE *fp;
	HVLSTR str;

	ps_push(hps,vlstr_newEx(szLanguageFile),L"rb");
	fp=ps_getFp(hps);
	if (fp){
		utf8_checkBOM(fp);
		str=vlstr_readLine(fp);
		ps_pop(hps);
		result=getLanguageEnum(vlstr_getData(str));
		vlstr_delete(str);
		if (result) return result;
	}
	ps_pop(hps);
	return lang_getUserLang();
}
