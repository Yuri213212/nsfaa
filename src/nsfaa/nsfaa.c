#include "nsfaa.h"

int main(){
	static int argc;
	static WCHAR **argv;

	int result;

	argv=CommandLineToArgvW(GetCommandLineW(),&argc);

	hps=ps_new();
	lang_init(hps);

	if (argc>1){
		fbuf=vlstr_newExw(argv[1]);
	}else{
		fbuf=NULL;
	}
	if (argc<2){
		MessageBoxW(NULL,szAbout,szAppName,MB_ICONINFORMATION);
		LocalFree(argv);
		return -1;
	}
	aas=aas_new(hps);
	errmsg=vlstr_new();
	result=aas_compile(aas,fbuf);
	aas_delete(aas);
	if (result){
		MessageBoxW(NULL,vlstr_getDataw(errmsg),szAppName,result<0?MB_ICONERROR:MB_ICONINFORMATION);
	}
	vlstr_delete(fbuf);
	vlstr_delete(errmsg);
	ps_delete(hps);
	LocalFree(argv);
	return result;
}
