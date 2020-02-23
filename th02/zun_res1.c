/* ReC98
 * -----
 * 1st part of ZUN_RES.COM. Initializes the resident structure and
 * configuration file required in order to run TH02.
 */

#pragma inline

#include <stddef.h>
#include "th02/th02.h"
#include "th02/snd/snd.h"

#pragma option -a1

char debug = 0;

void cfg_write(seg_t resident_sgm)
{
	static const huuma_options_t opts_default = {
		RANK_NORMAL, SND_BGM_FM, 3, 2, 0
	};
	static const char HUUMA_CFG[] = CFG_FN;

	const char *fn = HUUMA_CFG;
	huuma_options_t opts = opts_default;
	int handle = dos_axdx(0x3D02, fn);
	if(handle > 0) {
		dos_seek(handle, sizeof(opts), SEEK_SET);
	} else {
		handle = dos_create(fn, _A_ARCH);
		dos_write(handle, &opts, sizeof(opts));
	}
	dos_write(handle, &resident_sgm, sizeof(resident_sgm));
	dos_write(handle, &debug, sizeof(debug));
	dos_close(handle);
}

int main(int argc, const char **argv)
{
	int pascal scoredat_verify(void);

	static const char MIKOConfig[] = RES_ID;
	static const char LOGO[] =
		"\n"
		"\n"
		"���������^�p�@ �풓�v���O�����@ZUN_RES.com Version1.01       (c)zun 1997\n";
	static const char ERROR_SCOREDAT[] =
		"�n�C�X�R�A�t�@�C�������������́A������x���s���ĂˁB\n";
	static const char ERROR_NOT_RESIDENT[] =
		"�킽���A�܂����܂���悧\n\n";
	static const char REMOVED[] =
		"����Ȃ�A�܂�����炢����\n\n";
	static const char ERROR_UNKNOWN_OPTION[] =
		"����ȃI�v�V�����t�����Ă��A�����ł�����\n\n";
	static const char ERROR_ALREADY_RESIDENT[] =
		"�킽���A���łɂ��܂��悧\n\n";
	static const char ERROR_OUT_OF_MEMORY[] =
		"���܂���A�킽���̋��ꏊ���Ȃ��́I\n\n";
	static const char INITIALIZED[] =
		"����ł́A��낵�����肢���܂�\n\n";

	seg_t sgm;
	const char *res_id = MIKOConfig;
	int i;
	char far *resident;

	sgm = resdata_exist(res_id, RES_ID_STRLEN, RES_PARASIZE);
	dos_puts2(LOGO);
	graph_clear();
	// No, I tried all permutations of command-line switches,
	// gotos and returns, and no pure C solution seems to work!
	if(scoredat_verify() == 1) __asm {
		push offset ds:ERROR_SCOREDAT
		jmp error_puts
	}
	if(argc == 2) {
		#define arg1_is(capital, small) \
			(argv[1][0] == '-' || argv[1][0] == '/') \
			&& (argv[1][1] == (capital) || argv[1][1] == (small))
		if(arg1_is('R', 'r')) {
			if(!sgm) {
				dos_puts2(ERROR_NOT_RESIDENT);
asm				jmp error_ret
			}
			dos_free(sgm);
			dos_puts2(REMOVED);
			return 0;
		} else if(arg1_is('D', 'd')) {
			debug = 1;
		} else {
			dos_puts2(ERROR_UNKNOWN_OPTION);
			return 1;
		}
	}
	if(sgm) {
		dos_puts2(ERROR_ALREADY_RESIDENT);
		return 1;
	}
	sgm = resdata_create(res_id, RES_ID_STRLEN, RES_PARASIZE);
	if(!sgm) {
asm		push offset ds:ERROR_OUT_OF_MEMORY
error_puts:
asm		call near ptr dos_puts2
error_ret:
		return 1;
	}
	resident = MK_FP(sgm, 0);
	dos_puts2(INITIALIZED);
	for(i = offsetof(resident_t, stage); i < sizeof(resident_t); i++) {
		resident[i] = 0;
	}
	cfg_write(sgm);
	return 0;
}

#pragma codestring "\x00"
