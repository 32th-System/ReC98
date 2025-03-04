#include "th01/rank.h"

// Not *really* a cfg_options_t, since you'd expect that structure to contain
// the immutable contents of REIIDEN.CFG. However, [bombs] is in fact the
// *current* bomb count, and the .CFG value is saved to [credit_bombs]...
extern int8_t rank; // ACTUAL TYPE: rank_t
extern bgm_mode_t bgm_mode;
extern int8_t bombs;
extern int8_t credit_bombs;

extern int8_t stage_num;
extern bool bgm_change_blocked;

// Current gameplay frame plus resident_t::rand, without any frame_delay().
// Displayed as "rand" in the debug output, but can't be /* ZUN symbol */'d
// like that, due to obviously colliding with the C standard library function.
extern unsigned long frame_rand;

extern int8_t lives_extra;
extern bool first_stage_in_scene;
extern uint32_t score;
extern int8_t game_cleared; // ACTUAL TYPE: bool
