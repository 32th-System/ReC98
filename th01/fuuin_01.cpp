/* ReC98
 * -----
 * Code segment #1 of TH01's FUUIN.EXE
 */

#pragma option -O- -1 -Z-

#include <process.h>
#include <stdio.h>
#include "platform.h"
#include "master.hpp"
#include "th01/resident.hpp"
#include "th01/core/initexit.hpp"
#include "th01/end/end.hpp"
#include "th01/end/vars.hpp"
#include "th01/snd/mdrv2.h"
#include "th01/shiftjis/fns.hpp"

bool16 end_init(void)
{
	int i;
	resident_t __seg *sgm = ResData<resident_t>::exist(RES_ID);
	if(sgm) {
		resident_t* resident = sgm;
		if(resident->end_flag) {
			score = resident->score;
			continues_total = 0;
			end_flag = resident->end_flag;
			rank = resident->rank;
			score_highest = resident->score_highest;

			for(i = 0; i < SCENE_COUNT; i++) {
				continues_per_scene[i] = resident->continues_per_scene[i];
				resident->continues_per_scene[i] = 0;
				continues_total += continues_per_scene[i];
			}

			random_seed = resident->rand;
			start_lives_extra = resident->start_lives_extra;
			resident->score = 0;
			resident->continues_total = 0;
			resident->score_highest = 0;
		} else {
			puts("ERROR : end_flag is not ture !!");
			return false;
		}
	} else {
		puts("ERROR : cfg_id is not alloc!!");
		return false;
	}
	return true;
}

bool16 end_resident_clear(void)
{
	int i;
	resident_t __seg *sgm = ResData<resident_t>::exist(RES_ID);
	if(sgm) {
		resident_t* resident = sgm;
		resident->score = 0;
		resident->continues_total = 0;
		resident->end_flag = ES_NONE;
		resident->score_highest = 0;
		for(i = 0; i < SCENE_COUNT; i++) {
			resident->continues_per_scene[i] = 0;
		}
		resident->stage_id = 0;
	}
	return true;
}

void main(int argc, const char *argv[])
{
	if(!mdrv2_resident()) {
		return;
	}

	// Should really be checked, but eh, it's Real Mode...
	(argc);

	// That's a hidden ending preview feature!
	if(argv[1][0] != 't') {
		if(!end_init()) {
			return;
		}
	} else {
		if(argv[1][1] == '1') {
			end_flag = ES_MAKAI;
		} else {
			end_flag = ES_JIGOKU;
		}
	}

	// ZUN bug: Should be done conditionally based on resident_t::bgm_mode.
	// This is why you still get music during this binary despite disabling BGM
	// in the options.
	mdrv2_enable_if_board_installed();

	game_init();
	end_and_verdict_and_regist_animate();
	game_switch_binary();
	execl(BINARY_OP, BINARY_OP, nullptr);
}
