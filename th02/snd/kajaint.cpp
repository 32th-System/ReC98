#if (GAME == 5)
	#pragma option -zCSHARED_ -k-
#else
#pragma option -zCSHARED
#endif

#include "platform.h"
#include "x86real.h"
#include "libs/kaja/kaja.h"
extern "C" {
#if (GAME >= 4)
	#include "th04/snd/snd.h"
#else
	#include "th02/snd/snd.h"
#endif

int16_t DEFCONV snd_kaja_interrupt(int16_t ax)
{
	if(!snd_bgm_active()) {
		return _AX;
	}

	// TH04 should use snd_get_param() here, but doesn't....
	#if (GAME == 5)
		_AX = snd_get_param(ax);
	#else
		_AX = ax;
	#endif

	if(snd_bgm_is_fm()) {
		geninterrupt(PMD);
	} else {
		geninterrupt(MMD);
	}
	return _AX;
}
#if (GAME == 5)
	#pragma codestring "\x90"
#endif

}
