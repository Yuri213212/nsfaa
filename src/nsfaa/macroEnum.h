#include <string.h>

#define macroCount	25
#define macroNameLen	16

enum macroEnum{
	M_dummy=-1,
	M_bank,
	M_defnumlen,
	M_fds_bankid,
	M_fds_boottoid,
	M_fds_newside,
	M_image,
	M_import,
	M_include,
	M_nes_bankoffset,
	M_nes_battery,
	M_nes_mapper,
	M_nes_mirror,
	M_nsf_init,
	M_nsf_play,
	M_nsf_apu,
	M_nsf_artist,
	M_nsf_bankswitch,
	M_nsf_copyright,
	M_nsf_div2A03,
	M_nsf_div2A07,
	M_nsf_entry,
	M_nsf_expand,
	M_nsf_name,
	M_nsf_songs,
	M_nsf_trimend,
};

const char macros[macroCount][macroNameLen]={
	"bank",
	"defnumlen",
	"fds_bankid",
	"fds_boottoid",
	"fds_newside",
	"image",
	"import",
	"include",
	"nes_bankoffset",
	"nes_battery",
	"nes_mapper",
	"nes_mirror",
	"nsf_:init",
	"nsf_:play",
	"nsf_apu",
	"nsf_artist",
	"nsf_bankswitch",
	"nsf_copyright",
	"nsf_div2A03",
	"nsf_div2A07",
	"nsf_entry",
	"nsf_expand",
	"nsf_name",
	"nsf_songs",
	"nsf_trimend",
};

int m_bsch(char *s,int l,int r){
	int m,res;

	if (!s) return M_dummy;
	m=(l+r)/2;
	res=strcmp(s,macros[m]);
	if (!res) return m;
	if (res<0){
		if (m-1<l) return M_dummy;
		return m_bsch(s,l,m-1);
	}else{
		if (m+1>r) return M_dummy;
		return m_bsch(s,m+1,r);
	}
}

int getMacroEnum(char *s){
	if (!s) return M_dummy;
	return m_bsch(s,0,macroCount-1);
}
