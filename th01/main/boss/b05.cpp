/// Stage 5 Boss - SinGyoku
/// -----------------------

#include "platform.h"
#include "pc98.h"
#include "th01/math/subpixel.hpp"
#include "th01/main/vars.hpp"

#define boss_hp	singyoku_hp
#define boss_phase_frame	singyoku_phase_frame
#include "th01/main/boss/boss.hpp"

// Patterns
// --------

#define pattern_state	singyoku_pattern_state
extern union {
	int pellet_count;
	pixel_t speed_in_pixels;
	subpixel_t speed_in_subpixels;
	int unknown;
} pattern_state;
// --------

#define flash_colors	singyoku_flash_colors
#define invincible	singyoku_invincible
#define invincibility_frame	singyoku_invincibility_frame
#define initial_hp_rendered	singyoku_initial_hp_rendered
extern bool16 invincible;
extern int invincibility_frame;
extern bool16 initial_hp_rendered;

#define select_for_rank singyoku_select_for_rank
#include "th01/main/select_r.cpp"
