/// Jigoku Stage 20 Boss - Konngara
/// -------------------------------

extern "C" {
#include <stddef.h>
#include "platform.h"
#include "pc98.h"
#include "planar.h"
#include "master.hpp"
#include "th01/common.h"
#include "th01/math/area.hpp"
#include "th01/math/overlap.hpp"
#include "th01/math/subpixel.hpp"
#include "th01/math/vector.hpp"
#include "th01/hardware/frmdelay.h"
#include "th01/hardware/palette.h"
#include "th01/hardware/graph.h"
#include "th01/hardware/egc.h"
#include "th01/hardware/scrollup.hpp"
#include "th01/hardware/input.hpp"
#include "th01/snd/mdrv2.h"
#include "th01/main/playfld.hpp"
#include "th01/formats/grp.h"
#include "th01/formats/grz.h"
#include "th01/formats/pf.hpp"
#include "th01/formats/ptn.hpp"
#include "th01/formats/stagedat.hpp"
#include "th01/sprites/pellet.h"
#include "th01/sprites/shape8x8.hpp"
#include "th01/main/vars.hpp"
#include "th01/main/boss/entity_a.hpp"
#include "th01/main/stage/palette.hpp"
}
#include "th01/main/stage/stageobj.hpp"
#include "th01/main/shape.hpp"
#include "th01/main/player/player.hpp"
#include "th01/main/boss/boss.hpp"
#include "th01/main/boss/palette.hpp"
#include "th01/main/bullet/pellet.hpp"
#include "th01/main/bullet/laser_s.hpp"
#include "th01/main/hud/hp.hpp"

// Coordinates
// -----------

static const screen_x_t HEAD_LEFT = 280;
static const screen_y_t HEAD_TOP = 80;
static const screen_x_t FACE_LEFT = 280;
static const screen_y_t FACE_TOP = 128;
static const screen_x_t CUP_LEFT = 296;
static const screen_y_t CUP_TOP = 188;
static const screen_x_t LEFT_SLEEVE_LEFT = 290;
static const screen_y_t LEFT_SLEEVE_TOP = 200;
static const screen_x_t EYE_CENTER_X = 316;
static const screen_y_t EYE_BOTTOM = 140;

// Slash pattern spawners are moved on a triangle along these points.
static const screen_x_t SWORD_CENTER_X = 410;
static const screen_y_t SWORD_CENTER_Y = 70;
static const screen_x_t SLASH_4_CORNER_X = 432;
static const screen_y_t SLASH_4_CORNER_Y = 232;
static const screen_x_t SLASH_5_CORNER_X = 198;
static const screen_y_t SLASH_5_CORNER_Y = 198;

static const pixel_t CUP_W = 32;

static const screen_x_t CUP_RIGHT = (CUP_LEFT + CUP_W);
static const screen_x_t CUP_CENTER_X = (CUP_LEFT + (CUP_W / 2));
// -----------

// Other constants
// ----------------

static const pixel_t SLASH_DISTANCE_2_TO_4_X = (
	SLASH_4_CORNER_X - SWORD_CENTER_X
);
static const pixel_t SLASH_DISTANCE_2_TO_4_Y = (
	SLASH_4_CORNER_Y - SWORD_CENTER_Y
);
static const pixel_t SLASH_DISTANCE_5_TO_4_X = (
	SLASH_5_CORNER_X - SLASH_4_CORNER_X // yes, backwards
);
static const pixel_t SLASH_DISTANCE_5_TO_4_Y = (
	SLASH_5_CORNER_Y - SLASH_4_CORNER_Y // yes, backwards
);
static const pixel_t SLASH_DISTANCE_5_TO_6_X = (
	SLASH_5_CORNER_X - SWORD_CENTER_X
);
static const pixel_t SLASH_DISTANCE_5_TO_6_Y = (
	SLASH_5_CORNER_Y - SWORD_CENTER_Y
);

#define RAIN_G to_sp(0.0625f) /* Rain gravity */
// ----------------

#define pattern_state	konngara_pattern_state
#define flash_colors	konngara_flash_colors
#define invincible	konngara_invincible
#define invincibility_frame	konngara_invincibility_frame
#define initial_hp_rendered	konngara_initial_hp_rendered
extern union {
	int group; // pellet_group_t
	int interval;
	subpixel_t speed;
	pixel_t delta_x;
	int unused;
} pattern_state;
extern bool16 invincible;
extern int invincibility_frame;
extern bool initial_hp_rendered;

// Entities
// --------

enum face_direction_t {
	FD_RIGHT = 0,
	FD_LEFT = 1,
	FD_CENTER = 2,
	FD_COUNT = 3,
	FD_UNINITIALIZED = 9, // :zunpet:

	_face_direction_t_FORCE_INT16 = 0x7FFF
};

enum face_expression_t {
	FE_NEUTRAL = 0,
	FE_CLOSED = 1,
	FE_GLARE = 2,
	FE_AIM = 3,

	_face_expression_t_FORCE_INT16 = 0x7FFF
};

extern face_direction_t face_direction;
extern face_expression_t face_expression;
extern bool16 face_direction_can_change;

#define ent_head                	boss_entities[0]
#define ent_face_closed_or_glare	boss_entities[1]
#define ent_face_aim            	boss_entities[2]

#define head_put(direction) \
	ent_head.put_8(HEAD_LEFT, HEAD_TOP, direction);

#define face_aim_put(direction) \
	ent_face_aim.put_8(FACE_LEFT, FACE_TOP, direction);

#define face_put(expression, direction) \
	ent_face_closed_or_glare.put_8( \
		FACE_LEFT, FACE_TOP, (((expression - FE_CLOSED) * FD_COUNT) + direction) \
	);
// --------

// Snakes
// ------

static const int SNAKE_TRAIL_COUNT = 5;

inline screen_x_t snake_target_offset_left(const screen_x_t &to_left) {
	return (to_left + (PLAYER_W / 2) - (DIAMOND_W / 2));
}

#define SNAKE_HOMING_THRESHOLD \
	(PLAYFIELD_TOP + playfield_fraction_y(5, 7) - (DIAMOND_H / 2))

template <int SnakeCount> struct Snakes {
	screen_x_t left[SnakeCount][SNAKE_TRAIL_COUNT];
	screen_y_t top[SnakeCount][SNAKE_TRAIL_COUNT];
	pixel_t velocity_x[SnakeCount];
	pixel_t velocity_y[SnakeCount];
	bool16 target_locked[SnakeCount];
	screen_x_t target_left[SnakeCount];

	int count() const {
		return SnakeCount;
	}
};

#define snakes_wobbly_aim(snakes, snake_i, to_left, speed, tmp_angle) \
	tmp_angle = iatan2( \
		(player_center_y() - snakes.top[snake_i][0]), \
		(snake_target_offset_left(to_left) - snakes.left[snake_i][0]) \
	); \
	tmp_angle += ((rand() % 2) == 0) ? +0x28 : -0x28; \
	vector2( \
		(pixel_t far &)snakes.velocity_x[snake_i], \
		(pixel_t far &)snakes.velocity_y[snake_i], \
		speed, \
		tmp_angle \
	);

#define snakes_spawn_and_wobbly_aim( \
	snakes, snake_i, origin_x, origin_y, tmp_i, tmp_angle \
) \
	for(tmp_i = 0; tmp_i < SNAKE_TRAIL_COUNT; tmp_i++) { \
		snakes.left[snake_i][tmp_i] = origin_x; \
		snakes.top[snake_i][tmp_i] = origin_y; \
	} \
	snakes_wobbly_aim(snakes, snake_i, player_left, 6, tmp_angle)

#define snakes_unput_update_render(tmp_i, tmp_j, tmp_angle) \
	for(tmp_i = 0; tmp_i < snakes.count(); tmp_i++) { \
		/* Snake update */ \
		if(snakes.left[i][0] == -PIXEL_NONE) { \
			continue; \
		} \
		/* The last trail sprite is the only one we have to unblit here. */ \
		/* Since we forward-copy the coordinates for the remaining trail */ \
		/* segments, they're then drawn on top of previously blitted */ \
		/* sprites anyway. Nifty! */ \
		shape8x8_sloppy_unput( \
			snakes.left[tmp_i][SNAKE_TRAIL_COUNT - 1], \
			snakes.top[tmp_i][SNAKE_TRAIL_COUNT - 1] \
		); \
		\
		/* Render…? Before update? */ \
		for(tmp_j = (SNAKE_TRAIL_COUNT - 2); tmp_j >= 1; tmp_j--) { \
			shape8x8_diamond_put( \
				snakes.left[tmp_i][tmp_j], snakes.top[i][tmp_j], 9 \
			); \
		} \
		shape8x8_diamond_put(snakes.left[tmp_i][0], snakes.top[tmp_i][0], 7); \
		\
		/* Update */ \
		if(snakes.target_locked[tmp_i] == false) { \
			snakes.target_left[tmp_i] = player_left; \
		} \
		snakes_wobbly_aim(snakes, i, snakes.target_left[tmp_i], 7, angle); \
		if(snakes.top[tmp_i][0] > SNAKE_HOMING_THRESHOLD) { \
			snakes.target_locked[tmp_i] = true; \
		} \
		\
		/* Forward copy */ \
		for(tmp_j = (SNAKE_TRAIL_COUNT - 1); tmp_j >= 1; tmp_j--) { \
			snakes.left[tmp_i][tmp_j] = snakes.left[tmp_i][tmp_j - 1]; \
			snakes.top[tmp_i][tmp_j] = snakes.top[tmp_i][tmp_j - 1]; \
		} \
		\
		snakes.left[tmp_i][0] += snakes.velocity_x[tmp_i]; \
		snakes.top[tmp_i][0] += snakes.velocity_y[tmp_i]; \
		\
		/* Yes, that's a 30×30 hitbox around the player's center point if */ \
		/* the player is not sliding, only leaving out the edges. */ \
		if(overlap_xy_ltrb_lt_gt( \
			snakes.left[tmp_i][0], \
			snakes.top[tmp_i][0], \
			player_left, \
			(player_top + (player_sliding * 10)), \
			(player_right() - DIAMOND_W), \
			(player_bottom() - DIAMOND_H) \
		)) { \
			if(!player_invincible) { \
				done = true; \
			} \
		} \
	}

#define snakes_unput_all(snakes, tmp_i, tmp_j) \
	for(tmp_i = 0; tmp_i < snakes.count(); tmp_i++) { \
		for(j = 0; j < SNAKE_TRAIL_COUNT; j++) { \
			shape8x8_sloppy_unput(snakes.left[i][j], snakes.top[i][j]); \
		} \
	}
// ------

// File names
// ----------

// TODO: Inline (if used 1×), or make `const` (if used >1×), once Konngara is
// done
char SCROLL_BG_FN[] = "boss7_d1.grp";
char boss8_a1_grp[] = "boss8_a1.grp";
char ALICE_MDT[] = "ALICE.MDT";
#include "th01/shiftjis/fns.hpp"
char boss8_1_bos[] = "boss8_1.bos";
char boss8_e1_bos[] = "boss8_e1.bos";
char boss8_e2_bos[] = "boss8_e2.bos";
char GRZ_FN[] = "boss8.grz";
char boss8_d1_grp[] = "boss8_d1.grp";
char boss8_d2_grp[] = "boss8_d2.grp";
char boss8_d3_grp[] = "boss8_d3.grp";
char boss8_d4_grp[] = "boss8_d4.grp";
// ----------

#define select_for_rank konngara_select_for_rank
#include "th01/main/select_r.cpp"

void pellet_spawnray_unput_and_put(
	screen_x_t origin_x, vram_y_t origin_y,
	screen_x_t target_x, vram_y_t target_y,
	int col
)
{
	extern screen_x_t target_prev_x;
	extern vram_y_t target_prev_y;
	if(col == 99) {
		target_prev_x = -PIXEL_NONE;
		target_prev_y = -PIXEL_NONE;
		// Umm, shouldn't we unblit in this case?
		return;
	}
	if(
		(target_prev_x != -PIXEL_NONE) && (target_prev_y != -PIXEL_NONE) &&
		(target_prev_x >= 0) && (target_prev_x < RES_X) &&
		(target_prev_y >= 0) && (target_prev_y < RES_Y)
	) {
		graph_r_line_unput(origin_x, origin_y, target_prev_x, target_prev_y);
	}
	if(
		(target_x >= 0) && (target_x < RES_X) &&
		(target_y >= 0) && (target_y < RES_Y)
	) {
		graph_r_line(origin_x, origin_y, target_x, target_y, col);
	}
	target_prev_x = target_x;
	target_prev_y = target_y;
}

// Siddhaṃ seed syllables
// ----------------------

#define SIDDHAM_COL 0x9

inline void siddham_col_white(void) {
	z_palette_set_show(SIDDHAM_COL, 0xF, 0xF, 0xF);
}

#define siddham_col_white_in_step() \
	if(z_Palettes[SIDDHAM_COL].c.r > 0x0) { \
		z_Palettes[SIDDHAM_COL].c.r--; \
	} \
	if(z_Palettes[SIDDHAM_COL].c.g > 0x9) { \
		z_Palettes[SIDDHAM_COL].c.g--; \
	} \
	if(z_Palettes[SIDDHAM_COL].c.b > 0xA) { \
		z_Palettes[SIDDHAM_COL].c.b--; \
	} \
	z_palette_set_all_show(z_Palettes);
// ----------------------

void konngara_load_and_entrance(int8_t)
{
	int i;
	int j;
	int in_quarter;
	int ramp_col;
	int ramp_comp;
	int scroll_frame;

	pellet_interlace = true;

	text_fillca(' ', (TX_BLACK | TX_REVERSE));

	// graph_accesspage_func(0);
	grp_put_palette_show(SCROLL_BG_FN);
	stage_palette_set(z_Palettes);
	stageobjs_init_and_render(BOSS_STAGE);

	graph_accesspage_func(1);
	grp_put_palette_show(boss8_a1_grp);

	graph_accesspage_func(0);
	mdrv2_bgm_load(ALICE_MDT);
	mdrv2_se_load(SE_FN);
	mdrv2_bgm_play();

	z_palette_set_black(j, i);

	text_fillca(' ', TX_WHITE);
	ent_head.load(boss8_1_bos, 0);
	ent_face_closed_or_glare.load(boss8_e1_bos, 1);
	ent_face_aim.load(boss8_e2_bos, 2);

	// Decelerating scroll
	// -------------------

	#define quarters_remaining	i
	#define line_on_top       	j

	line_on_top = 0;
	quarters_remaining = 32; // Should be divisible by 4.
	in_quarter = 0;
	scroll_frame = 0;

	do {
		z_vsync_wait_and_scrollup(line_on_top);
		line_on_top += quarters_remaining;
		if((in_quarter == 0) && (line_on_top > ((RES_Y / 4) * 1))) {
			in_quarter++;
			quarters_remaining--;
		}
		if((in_quarter == 1) && (line_on_top > ((RES_Y / 4) * 2))) {
			in_quarter++;
			quarters_remaining--;
		}
		if((in_quarter == 2) && (line_on_top > ((RES_Y / 4) * 3))) {
			in_quarter++;
			quarters_remaining--;
		}
		if((in_quarter == 3) && (line_on_top > ((RES_Y / 4) * 4))) {
			in_quarter = 0;
			quarters_remaining--;
			line_on_top -= RES_Y;
		}
		if(quarters_remaining <= 0) {
			break;
		}
		if((scroll_frame % 8) == 0) {
			z_palette_black_in_step_to(ramp_col, ramp_comp, grp_palette)
		}
		scroll_frame++;
		frame_delay(1);
	} while(1);

	#undef line_on_top
	#undef quarters_remaining
	// -------------------

	z_vsync_wait_and_scrollup(0);
	grz_load_single(0, GRZ_FN, 0);
	grz_load_single(1, GRZ_FN, 1);
	grz_load_single(2, GRZ_FN, 2);
	grz_load_single(3, GRZ_FN, 3);
	grz_load_single(4, GRZ_FN, 4);
	grz_load_single(5, GRZ_FN, 5);
	grz_load_single(6, GRZ_FN, 6);

	frame_delay(40);

	// Shaking and panning
	// -------------------

	#define MAGNITUDE  	16
	#define frame      	i
	#define line_on_top	j

	// Shake (below)
	for(frame = 0; frame < 32; frame++) {
		z_vsync_wait_and_scrollup(
			(RES_Y + MAGNITUDE) - ((frame % 2) * (MAGNITUDE * 2))
		);
		if((frame % 8) == 0) {
			mdrv2_se_play(9);
		}
		frame_delay(1);
	}

	// "Pan" up to Konngara
	for(line_on_top = RES_Y; line_on_top >= 0; line_on_top -= (MAGNITUDE * 2)) {
		z_vsync_wait_and_scrollup(line_on_top);
		egc_copy_rows_1_to_0(line_on_top, (MAGNITUDE * 2));
		frame_delay(1);
	}

	// Shake
	for(frame = 0; frame < 32; frame++) {
		z_vsync_wait_and_scrollup(
			(RES_Y + MAGNITUDE) - ((frame % 2) * MAGNITUDE)
		);
		frame_delay(1);
	}

	#undef line_on_top
	#undef frame
	#undef MAGNITUDE
	// -------------------

	frame_delay(30);

	// Flashing Siddhaṃ seed syllables
	// -------------------------------

	siddham_col_white();
	grp_put_colorkey(boss8_d1_grp);
	grp_put_colorkey(boss8_d2_grp);
	grp_put_colorkey(boss8_d3_grp);
	grp_put_colorkey(boss8_d4_grp);

	for(j = 0; j < RGB4::Range; j++) {
		siddham_col_white_in_step();
		frame_delay(10);
	}
	graph_copy_page_back_to_front();
	// -------------------------------
}

void konngara_init(void)
{
	boss_palette_snap();
	void konngara_setup();
	konngara_setup();
}

void konngara_setup(void)
{
	boss_hp = 18;
	hud_hp_first_white = 16;
	hud_hp_first_redwhite = 10;
	boss_phase = 0;
	boss_phase_frame = 0;
	face_direction_can_change = true;
	face_expression = FE_NEUTRAL;
	face_direction = FD_CENTER;
}

void konngara_free(void)
{
	bos_entity_free(0);
	bos_entity_free(1);
	bos_entity_free(2);
	for(int i = 0; i < 7; i++) {
		grx_free(i);
	}
}

void face_direction_set_and_put(face_direction_t fd_new)
{
	if(!face_direction_can_change || (face_direction == fd_new)) {
		return;
	}
	graph_accesspage_func(1);	head_put(fd_new);
	graph_accesspage_func(0);	head_put(fd_new);
	if(face_expression == FE_AIM) {
		graph_accesspage_func(1);	face_aim_put(fd_new);
		graph_accesspage_func(0);	face_aim_put(fd_new);
	} else if(face_expression != FE_NEUTRAL) {
		graph_accesspage_func(1);	face_put(face_expression, fd_new);
		graph_accesspage_func(0);	face_put(face_expression, fd_new);
	}
	face_direction = fd_new;
}

void face_expression_set_and_put(face_expression_t fe_new)
{
	if(face_expression == fe_new) {
		return;
	}
	if(fe_new == FE_AIM) {
		graph_accesspage_func(1);	face_aim_put(face_direction);
		graph_accesspage_func(0);	face_aim_put(face_direction);
	} else if(fe_new != FE_NEUTRAL) {
		graph_accesspage_func(1);	face_put(fe_new, face_direction);
		graph_accesspage_func(0);	face_put(fe_new, face_direction);
	} else {
		graph_accesspage_func(1);	head_put(face_direction);
		graph_accesspage_func(0);	head_put(face_direction);
	}
	face_expression = fe_new;
}

void slash_put(int image)
{
	graph_accesspage_func(1);	grx_put(image);
	graph_accesspage_func(0);	grx_put(image);
}

void pattern_diamond_cross_to_edges_followed_by_rain(void)
{
	#define DIAMOND_COUNT 4
	#define DIAMOND_ORIGIN_X (PLAYFIELD_CENTER_X - (DIAMOND_W / 2))
	#define DIAMOND_ORIGIN_Y (PLAYFIELD_CENTER_Y + (DIAMOND_H / 2))

	int i;

	#define diamonds pattern0_diamonds
	extern struct {
		pixel_t velocity_bottomleft_x, velocity_topleft_x;
		pixel_t velocity_bottomleft_y, velocity_topleft_y;
		screen_x_t left[DIAMOND_COUNT];
		screen_y_t top[DIAMOND_COUNT];
	} diamonds;
	extern int frames_with_diamonds_at_edges;

	#define diamonds_unput(i) \
		for(i = 0; i < DIAMOND_COUNT; i++) { \
			shape8x8_sloppy_unput(diamonds.left[i], diamonds.top[i]); \
		}

	#define diamonds_put(i) \
		for(i = 0; i < DIAMOND_COUNT; i++) { \
			shape8x8_diamond_put(diamonds.left[i], diamonds.top[i], 9); \
		}

	if(boss_phase_frame == 10) {
		face_expression_set_and_put(FE_NEUTRAL);
	}
	if(boss_phase_frame < 100) {
		return;
	} else if(boss_phase_frame == 100) {
		// MODDERS: Just use a local variable.
		select_for_rank(pattern_state.group,
			PG_2_SPREAD_NARROW_AIMED,
			PG_3_SPREAD_NARROW_AIMED,
			PG_5_SPREAD_WIDE_AIMED,
			PG_5_SPREAD_NARROW_AIMED
		);

		vector2_between(
			DIAMOND_ORIGIN_X, DIAMOND_ORIGIN_Y,
			PLAYFIELD_LEFT, player_center_y(),
			diamonds.velocity_bottomleft_x, diamonds.velocity_bottomleft_y,
			7
		);
		vector2_between(
			DIAMOND_ORIGIN_X, DIAMOND_ORIGIN_Y,
			PLAYFIELD_LEFT, PLAYFIELD_TOP,
			diamonds.velocity_topleft_x, diamonds.velocity_topleft_y,
			7
		);

		for(i = 0; i < DIAMOND_COUNT; i++) {
			diamonds.left[i] = DIAMOND_ORIGIN_X;
			diamonds.top[i] = DIAMOND_ORIGIN_Y;
		}
		Pellets.add_group(
			(PLAYFIELD_LEFT + (PLAYFIELD_W / 2) - PELLET_W),
			(PLAYFIELD_TOP + playfield_fraction_y(8 / 21.0f) - (PELLET_H / 2)),
			static_cast<pellet_group_t>(pattern_state.group),
			to_sp(3.0f)
		);
		select_for_rank(pattern_state.interval, 18, 16, 14, 12);
		mdrv2_se_play(12);
	} else if(diamonds.left[0] > PLAYFIELD_LEFT) {
		diamonds_unput(i);
		diamonds.left[0] += diamonds.velocity_bottomleft_x;
		diamonds.top[0]  += diamonds.velocity_bottomleft_y;
		diamonds.left[1] -= diamonds.velocity_bottomleft_x;
		diamonds.top[1]  += diamonds.velocity_bottomleft_y;
		diamonds.left[2] += diamonds.velocity_topleft_x;
		diamonds.top[2]  += diamonds.velocity_topleft_y;
		diamonds.left[3] -= diamonds.velocity_topleft_x;
		diamonds.top[3]  += diamonds.velocity_topleft_y;
		if(diamonds.left[0] <= PLAYFIELD_LEFT) {
			diamonds.left[0] = PLAYFIELD_LEFT;
			diamonds.left[2] = PLAYFIELD_LEFT;
			diamonds.left[1] = (PLAYFIELD_RIGHT - DIAMOND_W);
			diamonds.left[3] = (PLAYFIELD_RIGHT - DIAMOND_W);
		} else {
			diamonds_put(i);
		}
		return;
	} else if(diamonds.top[0] > PLAYFIELD_TOP) {
		diamonds_unput(i);
		diamonds.top[0] -= 3;
		diamonds.top[1] -= 3;
		diamonds.left[2] += 6;
		diamonds.left[3] -= 6;
		if(diamonds.top[0] <= PLAYFIELD_TOP) {
			diamonds.top[0] = PLAYFIELD_TOP;
		} else {
			diamonds_put(i);
		}
		return;
	} else if(frames_with_diamonds_at_edges < 200) {
		frames_with_diamonds_at_edges++;
		if((frames_with_diamonds_at_edges % pattern_state.interval) == 0)  {
			#define speed to_sp(2.5f)
			screen_x_t from_left;
			screen_y_t from_top;
			screen_x_t to_left;
			screen_y_t to_top;
			unsigned char angle;

			from_left = PLAYFIELD_LEFT;
			from_top = (PLAYFIELD_TOP + playfield_rand_y(25 / 42.0f));
			// Should actually be
			// 	to_left = (PLAYFIELD_RIGHT - playfield_rand_x(5 / 8.0f));
			to_left = (PLAYFIELD_LEFT +
				playfield_rand_x(5 / 8.0f) + playfield_fraction_x(3 / 8.0f)
			);
			to_top = PLAYFIELD_BOTTOM;
			angle = iatan2((to_top - from_top), (to_left - from_left));
			Pellets.add_single(from_left, from_top, angle, speed, PM_NORMAL);

			from_left = (PLAYFIELD_RIGHT - PELLET_W);
			from_top = (PLAYFIELD_TOP + playfield_rand_y(25 / 42.0f));
			to_left = (PLAYFIELD_LEFT + playfield_rand_x( 5 /  8.0f));
			to_top = PLAYFIELD_BOTTOM;
			angle = iatan2((to_top - from_top), (to_left - from_left));
			Pellets.add_single(from_left, from_top, angle, speed, PM_NORMAL);

			from_top = PLAYFIELD_TOP;
			from_left = (PLAYFIELD_LEFT + playfield_rand_x());
			to_top = PLAYFIELD_BOTTOM;
			to_left = (PLAYFIELD_LEFT + playfield_rand_x());
			angle = iatan2((to_top - from_top), (to_left - from_left));
			Pellets.add_single(from_left, from_top, angle, speed, PM_NORMAL);

			from_top = PLAYFIELD_TOP;
			from_left = (PLAYFIELD_LEFT + playfield_rand_x());
			to_top = PLAYFIELD_BOTTOM;
			to_left = (PLAYFIELD_LEFT + playfield_rand_x());
			angle = iatan2((to_top - from_top), (to_left - from_left));
			Pellets.add_single(from_left, from_top, angle, speed, PM_NORMAL);

			from_top = PLAYFIELD_TOP;
			from_left = (PLAYFIELD_LEFT + playfield_rand_x());
			Pellets.add_group(from_left, from_top, PG_1_AIMED, speed);

			#undef speed
		}
		return;
	} else {
		boss_phase_frame = 0;
	}
	frames_with_diamonds_at_edges = 0;

	#undef diamonds_put
	#undef diamonds_unput
	#undef diamonds
	#undef DIAMOND_ORIGIN_Y
	#undef DIAMOND_ORIGIN_X
	#undef DIAMOND_COUNT
}

void pattern_symmetrical_from_cup_fire(unsigned char angle)
{
	Pellets.add_single(
		CUP_RIGHT, CUP_TOP, angle, pattern_state.speed, PM_NORMAL
	);
	Pellets.add_single(
		CUP_LEFT,  CUP_TOP, angle, pattern_state.speed, PM_NORMAL
	);
}

void pattern_symmetrical_from_cup(void)
{
	#define angle pattern1_angle
	#define unused pattern1_unused
	extern unsigned char angle;
	extern int16_t unused;

	if(boss_phase_frame == 10) {
		face_expression_set_and_put(FE_CLOSED);
	}
	if(boss_phase_frame < 100) {
		return;
	}
	if(boss_phase_frame == 100) {
		angle = 0x40;
		unused = -1;
		select_for_rank(pattern_state.speed,
			to_sp(5.0f), to_sp(5.0f), to_sp(6.0f), to_sp(7.0f)
		);
	}
	if((boss_phase_frame < 140) && ((boss_phase_frame % 8) == 0)) {
		pattern_symmetrical_from_cup_fire(0x40);
		return;
	}
	if((boss_phase_frame < 220) && ((boss_phase_frame % 8) == 0)) {
		angle += 0x05;
		pattern_symmetrical_from_cup_fire(angle);
		pattern_symmetrical_from_cup_fire(0x80 - angle);
		return;
	}
	if((boss_phase_frame < 300) && ((boss_phase_frame % 8) == 0)) {
		angle -= 0x0C;
		pattern_symmetrical_from_cup_fire(angle);
		pattern_symmetrical_from_cup_fire(0x80 - angle);
	}
	if(boss_phase_frame >= 300) {
		boss_phase_frame = 0;
	}

	#undef unused
	#undef angle
}

void pattern_two_homing_snakes_and_semicircle_spreads(void)
{
	#define snakes pattern2_snakes
	extern Snakes<2> snakes;

	int i;
	int j;
	screen_x_t pellet_left;
	screen_y_t pellet_top;
	unsigned char angle;

	if(boss_phase_frame == 10) {
		face_expression_set_and_put(FE_GLARE);
	}
	if(boss_phase_frame < 100) {
		return;
	}
	if(boss_phase_frame == 100) {
		snakes.target_locked[0] = false;
		snakes.target_locked[1] = false;
		snakes_spawn_and_wobbly_aim(snakes, 0, CUP_CENTER_X, CUP_TOP, i, angle);
		snakes.left[1][0] = -PIXEL_NONE;
		mdrv2_se_play(12);
		return;
	}
	if(boss_phase_frame < 500) {
		snakes_unput_update_render(i, j, angle)
		if(boss_phase_frame == 150) {
			snakes_spawn_and_wobbly_aim(
				snakes, 1, CUP_CENTER_X, CUP_TOP, i, angle
			);
			mdrv2_se_play(12);
		}

		if(
			(boss_phase_frame > (240 - (rank * 40))) &&
			((boss_phase_frame % 40) == 0)
		) {
			enum {
				SPREAD = 10,
			};
			pixel_t velocity_x;
			pixel_t velocity_y;
			Subpixel speed;

			angle = (rand() % (0x80 / SPREAD));
			pellet_left =
				((boss_phase_frame % 120) ==  0) ? SWORD_CENTER_X :
				((boss_phase_frame % 120) == 40) ? EYE_CENTER_X :
				/*boss_phase_frame % 120  == 80 */ CUP_CENTER_X;
			pellet_top =
				((boss_phase_frame % 120) ==  0) ? SWORD_CENTER_Y :
				((boss_phase_frame % 120) == 40) ? EYE_BOTTOM :
				/*boss_phase_frame % 120  == 80 */ CUP_TOP;

			for(i = 0; i < SPREAD; i++) {
				speed.v = (to_sp(2.5f) + (
					((i % 4) == 0) ? to_sp(0.0f) :
					((i % 4) == 1) ? to_sp(1.0f) :
					((i % 4) == 2) ? to_sp(0.0f) :
					/*i % 4  == 3 */ to_sp(2.0f)
				));

				// That result is never used?
				vector2(velocity_x, velocity_y, speed, angle);

				Pellets.add_single(
					pellet_left, pellet_top, (0x80 - angle), speed, PM_NORMAL
				);
				Pellets.add_single(
					pellet_left, pellet_top, angle, speed, PM_NORMAL
				);
				angle += (0x80 / SPREAD);
			}
			mdrv2_se_play(7);
		};
	} else {
		snakes_unput_all(snakes, i, j);
		boss_phase_frame = 0;
	}

	#undef snakes
}

void pattern_aimed_rows_from_top(void)
{
	enum {
		DIAMOND_SPEED = 8,
		ROW_MARGIN = (PLAYFIELD_W / 10),
	};
	#define diamond_velocity pattern3_diamond_velocity
	#define diamond_left pattern3_diamond_left
	#define diamond_top pattern3_diamond_top
	#define diamond_direction pattern3_diamond_direction
	#define pellet_speed pattern3_pellet_speed

	extern point_t diamond_velocity;
	// screen_point_t would generate too good ASM here
	extern screen_x_t diamond_left;
	extern screen_y_t diamond_top;
	extern enum {
		RIGHT = 0,
		LEFT = 1,
		DOWN_START = 2,
		DOWN_END = (DOWN_START + (ROW_MARGIN / DIAMOND_SPEED)),
		TO_INITIAL_POSITION = 99,

		_diamond_direction_t_FORCE_INT16 = 0x7FFF
	} diamond_direction;
	extern Subpixel pellet_speed;

	if(boss_phase_frame == 10) {
		face_expression_set_and_put(FE_NEUTRAL);
	}
	if(boss_phase_frame < 100) {
		return;
	}
	if(boss_phase_frame == 100) {
		vector2_between(
			LEFT_SLEEVE_LEFT, LEFT_SLEEVE_TOP,
			(PLAYFIELD_LEFT + ROW_MARGIN), PLAYFIELD_TOP,
			diamond_velocity.x, diamond_velocity.y,
			(DIAMOND_SPEED * 2)
		);
		diamond_left = LEFT_SLEEVE_LEFT;
		diamond_top = LEFT_SLEEVE_TOP;
		pellet_speed.set(3.0f);
		mdrv2_se_play(12);
		diamond_direction = TO_INITIAL_POSITION;
		select_for_rank(pattern_state.interval, 12, 10, 8, 6);
		return;
	}
	if(diamond_direction == TO_INITIAL_POSITION) {
		shape8x8_sloppy_unput(diamond_left, diamond_top);
		diamond_left += diamond_velocity.x;
		diamond_top += diamond_velocity.y;

		if(diamond_top <= PLAYFIELD_TOP) {
			diamond_left = (PLAYFIELD_LEFT + ROW_MARGIN);
			diamond_top = PLAYFIELD_TOP;
			diamond_direction = RIGHT;
			return;
		}
		shape8x8_diamond_put(diamond_left, diamond_top, 9);
	} else if(diamond_direction < TO_INITIAL_POSITION) {
		shape8x8_sloppy_unput(diamond_left, diamond_top);

		// That's quite the roundabout way of saying "-8, 0, or +8"...
		diamond_left += (DIAMOND_SPEED + (
			(diamond_direction == RIGHT) ?   0 :
			(diamond_direction ==  LEFT) ? (-DIAMOND_SPEED * 2) :
			/*            >= DOWN_START */  -DIAMOND_SPEED
		));
		if(diamond_direction >= DOWN_START) {
			diamond_top += DIAMOND_SPEED;
			reinterpret_cast<int &>(diamond_direction)++;
			if(diamond_direction >= DOWN_END) {
				diamond_direction = (diamond_left > PLAYFIELD_CENTER_X)
					? LEFT
					: RIGHT;
			}
		}
		if(diamond_left > (PLAYFIELD_RIGHT - ROW_MARGIN)) {
			diamond_direction = DOWN_START;
			diamond_left = (PLAYFIELD_RIGHT - ROW_MARGIN);
		} else if(diamond_left < (PLAYFIELD_LEFT + ROW_MARGIN)) {
			diamond_direction = DOWN_START;
			diamond_left = (PLAYFIELD_LEFT + ROW_MARGIN);
		}
		if(diamond_top >= (PLAYFIELD_TOP + (ROW_MARGIN * 3))) {
			boss_phase_frame = 0;
			return;
		}
		if(diamond_direction < DOWN_START) {
			if((boss_phase_frame % pattern_state.interval) == 0) {
				Pellets.add_group(
					diamond_left, diamond_top, PG_1_AIMED, pellet_speed
				);
				pellet_speed += 0.125f;
			}
		}
		shape8x8_diamond_put(diamond_left, diamond_top, 6);
	}

	#undef pellet_speed
	#undef diamond_direction
	#undef diamond_top
	#undef diamond_left
	#undef diamond_velocity
}

void pattern_aimed_spray_from_cup(void)
{
	#define spray_offset pattern4_spray_offset
	#define angle pattern4_angle
	#define spray_delta pattern4_spray_delta
	#define frames_in_current_direction pattern4_frames_in_current_direction

	extern unsigned char spray_offset;
	extern unsigned char angle; // should be local
	extern int spray_delta; // should be unsigned char
	extern int frames_in_current_direction;

	if(boss_phase_frame == 10) {
		face_expression_set_and_put(FE_CLOSED);
	}
	if(boss_phase_frame < 100) {
		return;
	}
	if(boss_phase_frame == 100) {
		spray_offset = 0x20;
		spray_delta = -0x08;
		frames_in_current_direction = 0;
		select_for_rank(pattern_state.interval, 5, 4, 3, 2);
	}
	if((boss_phase_frame % pattern_state.interval) == 0) {
		// Yes, the point from which these are aimed to the top-left player
		// coordinate is quite a bit away from where they're actually fired,
		// leading to some quite imperfect aiming. Probably done on purpose
		// though, and largely mitigated by the spraying motion anyway.
		angle = iatan2(
			(player_top - (CUP_TOP - 4)), (player_left - (CUP_CENTER_X - 34))
		);
		angle += spray_offset;
		spray_offset += spray_delta;
		frames_in_current_direction++;
		if(frames_in_current_direction > 8) {
			spray_delta *= -1;
			frames_in_current_direction = 0;
		}
		Pellets.add_single(
			CUP_CENTER_X, CUP_TOP, angle, to_sp(3.0f), PM_NORMAL
		);
	}
	if(boss_phase_frame >= 700) {
		boss_phase_frame = 0;
	}

	#undef frames_in_current_direction
	#undef spray_delta
	#undef angle
	#undef spray_offset
}

void pattern_four_homing_snakes(void)
{
	#define snakes pattern5_snakes
	extern Snakes<4> snakes;

	int i;
	int j;
	unsigned char angle;

	if(boss_phase_frame == 10) {
		face_expression_set_and_put(FE_GLARE);
	}
	if(boss_phase_frame < 100) {
		return;
	}
	if(boss_phase_frame == 100) {
		for(i = 0; i < snakes.count(); i++) {
			snakes.target_locked[i] = false;
		}
		snakes_spawn_and_wobbly_aim(snakes, 0, CUP_CENTER_X, CUP_TOP, i, angle);
		for(i = 1; i < snakes.count(); i++) {
			snakes.left[i][0] = -PIXEL_NONE;
		}
		konngara_select_for_rank(pattern_state.unused, 18, 16, 14, 12);
		mdrv2_se_play(12);
		return;
	}
	if(boss_phase_frame < 400) {
		snakes_unput_update_render(i, j, angle);
		if(boss_phase_frame == 150) {
			snakes_spawn_and_wobbly_aim(
				snakes, 1, CUP_CENTER_X, CUP_TOP, i, angle
			);
			mdrv2_se_play(12);
		}
		if(boss_phase_frame == 200) {
			snakes_spawn_and_wobbly_aim(
				snakes, 2, LEFT_SLEEVE_LEFT, LEFT_SLEEVE_TOP, i, angle
			);
			mdrv2_se_play(12);
		}
		if(boss_phase_frame == 250) {
			snakes_spawn_and_wobbly_aim(
				snakes, 3, LEFT_SLEEVE_LEFT, LEFT_SLEEVE_TOP, i, angle
			);
			mdrv2_se_play(12);
		}
	} else {
		snakes_unput_all(snakes, i, j);
		boss_phase_frame = 0;
	}

	#undef snakes
}

inline void swordray_unput_put_and_move(
	screen_x_t& end_x, screen_x_t& end_y, screen_x_t delta_x, screen_y_t delta_y
) {
	pellet_spawnray_unput_and_put(
		SWORD_CENTER_X, SWORD_CENTER_Y, end_x, end_y, 6
	);
	// Gimme those original instructions!
	if(delta_x < 0) { end_x -= -delta_x; } else { end_x += delta_x; }
	if(delta_y < 0) { end_y -= -delta_y; } else { end_y += delta_y; }
}

inline void swordray_unput(const screen_x_t& end_x, const screen_x_t& end_y) {
	graph_r_line_unput(SWORD_CENTER_X, SWORD_CENTER_Y, end_x, end_y);
}

void pattern_rain_from_edges(void)
{
	#define end_x pattern6_end_x
	#define end_y pattern6_end_y
	#define unused pattern6_unused
	extern screen_x_t end_x;
	extern screen_y_t end_y;
	extern int unused;

	enum {
		SPAWNRAY_SPEED = 8,
		FRAMES_VERTICAL = 25,
		FRAMES_HORIZONTAL = (PLAYFIELD_W / SPAWNRAY_SPEED),

		KEYFRAME_0 = 100,
		KEYFRAME_1 = (KEYFRAME_0 + FRAMES_VERTICAL), // up
		KEYFRAME_2 = (KEYFRAME_1 + FRAMES_HORIZONTAL), // right
		KEYFRAME_3 = (KEYFRAME_2 + FRAMES_VERTICAL), // down
		KEYFRAME_4 = (KEYFRAME_3 + FRAMES_VERTICAL), // up
		KEYFRAME_5 = (KEYFRAME_4 + FRAMES_HORIZONTAL), // left
		KEYFRAME_6 = (KEYFRAME_5 + FRAMES_VERTICAL), // down
	};

	if(boss_phase_frame == 10) {
		face_expression_set_and_put(FE_AIM);
	}
	if(boss_phase_frame < KEYFRAME_0) {
		return;
	}
	if(boss_phase_frame == KEYFRAME_0) {
		end_x = PLAYFIELD_LEFT;
		end_y = (PLAYFIELD_TOP + (SPAWNRAY_SPEED * FRAMES_VERTICAL));
		unused = 1;
		select_for_rank(pattern_state.interval, 5, 3, 2, 2);
	}
	if(boss_phase_frame < KEYFRAME_1) {
		swordray_unput_put_and_move(end_x, end_y, 0, -SPAWNRAY_SPEED);
	} else if(boss_phase_frame == KEYFRAME_1) {
		end_x = PLAYFIELD_LEFT;
		end_y = PLAYFIELD_TOP;
		swordray_unput_put_and_move(end_x, end_y, +SPAWNRAY_SPEED, 0);

		unused = 0;
	} else if(boss_phase_frame < KEYFRAME_2) {
		swordray_unput_put_and_move(end_x, end_y, +SPAWNRAY_SPEED, 0);
	} else if(boss_phase_frame == KEYFRAME_2) {
		end_x = (PLAYFIELD_RIGHT - 1);
		end_y = PLAYFIELD_TOP;
		swordray_unput_put_and_move(end_x, end_y, 0, +SPAWNRAY_SPEED);

		unused = 2;
	} else if(boss_phase_frame < KEYFRAME_3) {
		swordray_unput_put_and_move(end_x, end_y, 0, +SPAWNRAY_SPEED);
	} else if(boss_phase_frame == KEYFRAME_3) {
		end_x = (PLAYFIELD_RIGHT - 1);
		end_y = (PLAYFIELD_TOP + (SPAWNRAY_SPEED * FRAMES_VERTICAL));
		swordray_unput_put_and_move(end_x, end_y, 0, -SPAWNRAY_SPEED);

		unused = 2;
	} else if(boss_phase_frame < KEYFRAME_4) {
		swordray_unput_put_and_move(end_x, end_y, 0, -SPAWNRAY_SPEED);
	} else if(boss_phase_frame == KEYFRAME_4) {
		end_x = (PLAYFIELD_RIGHT - 1);
		end_y = PLAYFIELD_TOP;
		swordray_unput_put_and_move(end_x, end_y, -SPAWNRAY_SPEED, 0);

		unused = 0;
	} else if(boss_phase_frame < KEYFRAME_5) {
		swordray_unput_put_and_move(end_x, end_y, -SPAWNRAY_SPEED, 0);
	} else if(boss_phase_frame == KEYFRAME_5) {
		end_x = PLAYFIELD_LEFT;
		end_y = PLAYFIELD_TOP;
		swordray_unput_put_and_move(end_x, end_y, 0, +SPAWNRAY_SPEED);

		unused = 1;
	} else if(boss_phase_frame < KEYFRAME_6) {
		swordray_unput_put_and_move(end_x, end_y, 0, +SPAWNRAY_SPEED);
	} else if(boss_phase_frame == KEYFRAME_6) {
		// Wait, what, changing the end point of the ray immediately before
		// unblitting?! Technically wrong, but since line unblitting uses
		// 32-dot chunks anyway, this doesn't leave any visible glitches.
		end_y -= SPAWNRAY_SPEED;
		swordray_unput(end_x, end_y);
		boss_phase_frame = 0;
	}
	if((boss_phase_frame % 10) == 0) {
		mdrv2_se_play(6);
	}
	if((boss_phase_frame % pattern_state.interval) == 0) {
		Pellets.add_single(
			end_x, end_y, (rand() & 0x7F), to_sp(2.0f), PM_GRAVITY, RAIN_G
		);
		Pellets.add_single(
			end_x, end_y, (rand() & 0x7F), to_sp(2.0f), PM_GRAVITY, RAIN_G
		);
	}

	#undef unused
	#undef end_y
	#undef end_x
}

enum slash_cel_frame_t {
	SLASH_0_FRAME = 50,
	SLASH_1_FRAME = 60,
	SLASH_2_FRAME = 100,
	SLASH_3_FRAME = 120,
	SLASH_4_FRAME = 140,
	SLASH_4_5_FRAME = 150,
	SLASH_5_FRAME = 160,
	SLASH_6_FRAME = 170,

	SLASH_FRAMES_FROM_2_TO_4 = (SLASH_4_FRAME - SLASH_2_FRAME),
	 // Yes, also backwards
	SLASH_FRAMES_FROM_5_TO_4_5 = (SLASH_4_5_FRAME - SLASH_5_FRAME),
	SLASH_FRAMES_FROM_4_5_TO_6 = (SLASH_6_FRAME - SLASH_4_5_FRAME),
};

void slash_animate(void)
{
	// MODDERS: Just use a switch.
	#define boss_phase_at_frame(frame) \
		boss_phase_frame < frame) return; if(boss_phase_frame == frame

	if(boss_phase_at_frame(SLASH_0_FRAME)) {
		slash_put(0);
		mdrv2_se_play(8);
	}
	if(boss_phase_at_frame(SLASH_1_FRAME)) {
		slash_put(1);
	}
	if(boss_phase_at_frame(SLASH_2_FRAME)) {
		slash_put(2);
	}
	if(boss_phase_at_frame(SLASH_3_FRAME)) {
		slash_put(3);
	}
	if(boss_phase_at_frame(SLASH_4_FRAME)) {
		slash_put(4);
	}
	if(boss_phase_at_frame(SLASH_5_FRAME)) {
		slash_put(5);
	}
	if(boss_phase_at_frame(SLASH_6_FRAME)) {
		slash_put(6);
	}
	if(boss_phase_frame > 200) {
		boss_phase_frame = 0;
		face_direction_can_change = true;
	}

	#undef boss_phase_at_frame
}

#define slash_spawner_step_from_2_to_4(left, top, steps) \
	left += (SLASH_DISTANCE_2_TO_4_X / (SLASH_FRAMES_FROM_2_TO_4 / steps)); \
	top  += (SLASH_DISTANCE_2_TO_4_Y / (SLASH_FRAMES_FROM_2_TO_4 / steps));

#define slash_spawner_step_from_4_to_4_5(left, top, steps) \
	left -= (SLASH_DISTANCE_5_TO_4_X / (SLASH_FRAMES_FROM_5_TO_4_5 / steps)); \
	top  -= (SLASH_DISTANCE_5_TO_4_Y / (SLASH_FRAMES_FROM_5_TO_4_5 / steps));

#define slash_spawner_step_from_4_5_to_6(left, top, steps) \
	left -= (SLASH_DISTANCE_5_TO_6_X / (SLASH_FRAMES_FROM_4_5_TO_6 / steps)); \
	top  -= (SLASH_DISTANCE_5_TO_6_Y / (SLASH_FRAMES_FROM_4_5_TO_6 / steps));

inline void slash_rain_fire(
	const screen_x_t& left, const screen_y_t& top, subpixel_t speed
) {
	Pellets.add_single(left, top, (rand() % 0x7F), speed, PM_GRAVITY, RAIN_G);
	Pellets.add_single(left, top, (rand() % 0x7F), speed, PM_GRAVITY, RAIN_G);
}

void pattern_slash_rain(void)
{
	#define spawner_left pattern7_spawner_left
	#define spawner_top pattern7_spawner_top
	extern screen_x_t spawner_left;
	extern screen_y_t spawner_top;

	if(boss_phase_frame == 10) {
		face_direction_set_and_put(FD_CENTER);
		face_expression_set_and_put(FE_CLOSED);
		face_direction_can_change = false;
		spawner_left = SWORD_CENTER_X;
		spawner_top = SWORD_CENTER_Y;
		select_for_rank(pattern_state.interval, 5, 3, 2, 1);
	}

	slash_animate();

	if(boss_phase_frame < SLASH_2_FRAME) {
		return;
	}
	if(
		(boss_phase_frame < SLASH_4_FRAME) &&
		((boss_phase_frame % pattern_state.interval) == 0)
	) {
		slash_rain_fire(spawner_left, spawner_top, to_sp(0.0f));
		slash_spawner_step_from_2_to_4(
			spawner_left, spawner_top, pattern_state.interval
		);
	}

	if(boss_phase_frame == SLASH_4_FRAME) {
		spawner_left = SLASH_4_CORNER_X;
		spawner_top = SLASH_4_CORNER_Y;
		// Originally meant to be the step interval between cels 4 and 5?
		select_for_rank(pattern_state.unused, 3, 2, 2, 2);
	}
	if(boss_phase_frame < SLASH_4_FRAME) {
		return;
	}
	if(boss_phase_frame < SLASH_4_5_FRAME) {
		slash_rain_fire(spawner_left, spawner_top, to_sp(0.0f));
		slash_spawner_step_from_4_to_4_5(spawner_left, spawner_top, 1);
	}

	if(boss_phase_frame == SLASH_4_5_FRAME) {
		spawner_left = SLASH_5_CORNER_X;
		spawner_top = SLASH_5_CORNER_Y;
		select_for_rank(pattern_state.interval, 3, 2, 1, 1);
	}
	if(boss_phase_frame < SLASH_4_5_FRAME) {
		return;
	}
	if(
		(boss_phase_frame < SLASH_6_FRAME) &&
		((boss_phase_frame % pattern_state.interval) == 0)
	) {
		slash_rain_fire(spawner_left, spawner_top, to_sp(0.0f));
		slash_spawner_step_from_4_5_to_6(
			spawner_left, spawner_top, pattern_state.interval
		);
	}

	#undef spawner_top
	#undef spawner_left
}

inline void slash_triangular_fire(
	const screen_x_t& left, const screen_y_t& top, const subpixel_t& speed
) {
	Pellets.add_single(left, top, 0x20, speed, PM_NORMAL);
	Pellets.add_single(left, top, 0x60, speed, PM_NORMAL);
}

void pattern_slash_triangular(void)
{
	#define spawner_left pattern8_spawner_left
	#define spawner_top pattern8_spawner_top
	extern screen_x_t spawner_left;
	extern screen_y_t spawner_top;

	if(boss_phase_frame == 10) {
		face_direction_set_and_put(FD_CENTER);
		face_expression_set_and_put(FE_AIM);
		face_direction_can_change = false;
		spawner_left = SWORD_CENTER_X;
		spawner_top = SWORD_CENTER_Y;
		select_for_rank(
			pattern_state.speed,
			to_sp(2.0f), to_sp(3.0f), to_sp(4.0f), to_sp(4.5f)
		);
	}

	slash_animate();

	if(boss_phase_frame < SLASH_2_FRAME) {
		return;
	}
	if((boss_phase_frame < SLASH_4_FRAME) && ((boss_phase_frame % 3) == 0)) {
		slash_triangular_fire(spawner_left, spawner_top, pattern_state.speed);
		slash_spawner_step_from_2_to_4(spawner_left, spawner_top, 3);
	}

	if(boss_phase_frame == SLASH_4_FRAME) {
		spawner_left = SLASH_4_CORNER_X;
		spawner_top = SLASH_4_CORNER_Y;
	}
	if(boss_phase_frame < SLASH_4_FRAME) {
		return;
	}
	if(boss_phase_frame < SLASH_4_5_FRAME) {
		slash_triangular_fire(spawner_left, spawner_top, pattern_state.speed);
		slash_spawner_step_from_4_to_4_5(spawner_left, spawner_top, 1);
	}

	if(boss_phase_frame == SLASH_4_5_FRAME) {
		spawner_left = SLASH_5_CORNER_X;
		spawner_top = SLASH_5_CORNER_Y;
	}
	if(boss_phase_frame < SLASH_4_5_FRAME) {
		return;
	}
	if((boss_phase_frame < SLASH_6_FRAME) && ((boss_phase_frame % 2) == 0)) {
		slash_triangular_fire(spawner_left, spawner_top, pattern_state.speed);
		slash_spawner_step_from_4_5_to_6(spawner_left, spawner_top, 2);
	}

	#undef spawner_top
	#undef spawner_left
}

void pattern_lasers_and_3_spread(void)
{
	#define target_left pattern9_target_left
	#define target_y pattern9_target_y
	#define right_to_left pattern9_right_to_left

	// These have no reason to be static.
	extern screen_x_t target_left;
	extern screen_y_t target_y;

	extern bool16 right_to_left;

	enum {
		INTERVAL = 10,
	};

	if(boss_phase_frame == 10) {
		face_expression_set_and_put(FE_AIM);
	}
	if(boss_phase_frame < 100) {
		return;
	}
	if(boss_phase_frame == 100) {
		right_to_left = (rand() % 2);

		// Divisor = number of lasers that are effectively fired.
		select_for_rank(pattern_state.delta_x,
			(PLAYFIELD_W / 5),
			(PLAYFIELD_W / 6.66),
			(PLAYFIELD_W / 8),
			(PLAYFIELD_W / 10)
		);
	}
	if((boss_phase_frame % INTERVAL) == 0) {
		if(right_to_left == 0) {
			target_left = (PLAYFIELD_LEFT + (
				((boss_phase_frame - 100) / INTERVAL) * pattern_state.delta_x
			));
		} else {
			target_left = (PLAYFIELD_RIGHT - (
				((boss_phase_frame - 100) / INTERVAL) * pattern_state.delta_x
			));
		}
		target_y = PLAYFIELD_BOTTOM;

		// Quite a roundabout way of preventing a buffer overflow, but fine.
		shootout_lasers[
			(boss_phase_frame / INTERVAL) % SHOOTOUT_LASER_COUNT
		].spawn(
			SWORD_CENTER_X, SWORD_CENTER_Y,
			target_left, target_y,
			(to_sp(8.5f) / 2), 7, 30, 5
		);
		mdrv2_se_play(6);

		if(
			((right_to_left == false) && (target_left >= PLAYFIELD_RIGHT)) ||
			((right_to_left == true)  && (target_left <= PLAYFIELD_LEFT))
		) {
			boss_phase_frame = 0;
		}

		Pellets.add_group(
			SWORD_CENTER_X, SWORD_CENTER_Y, PG_3_SPREAD_WIDE_AIMED, to_sp(4.5f)
		);
	}

	#undef right_to_left
	#undef target_y
	#undef target_left
}

inline void slash_aimed_fire(
	const screen_x_t& left, const screen_y_t& top, const subpixel_t& speed
) {
	Pellets.add_group(left, top, PG_1_AIMED, speed);
}

void pattern_slash_aimed(void)
{
	#define spawner_left pattern10_spawner_left
	#define spawner_top pattern10_spawner_top
	extern screen_x_t spawner_left;
	extern screen_y_t spawner_top;

	if(boss_phase_frame == 10) {
		face_direction_set_and_put(FD_CENTER);
		face_expression_set_and_put(FE_AIM);
		face_direction_can_change = false;
		spawner_left = SWORD_CENTER_X;
		spawner_top = SWORD_CENTER_Y;
		konngara_select_for_rank(pattern_state.speed,
			to_sp(4.0f), to_sp(5.0f), to_sp(5.5f), to_sp(6.0f)
		);
	}

	slash_animate();
	if(boss_phase_frame < SLASH_2_FRAME) {
		return;
	}
	if((boss_phase_frame < SLASH_4_FRAME) && ((boss_phase_frame % 3) == 0)) {
		slash_aimed_fire(spawner_left, spawner_top, pattern_state.speed);
		slash_spawner_step_from_2_to_4(spawner_left, spawner_top, 3);
	}

	if(boss_phase_frame == SLASH_4_FRAME) {
		spawner_left = SLASH_4_CORNER_X;
		spawner_top = SLASH_4_CORNER_Y;
	}
	if(boss_phase_frame < SLASH_4_FRAME) {
		return;
	}
	if(boss_phase_frame < SLASH_4_5_FRAME) {
		slash_aimed_fire(spawner_left, spawner_top, pattern_state.speed);
		slash_spawner_step_from_4_to_4_5(spawner_left, spawner_top, 1);
	}

	if(boss_phase_frame == SLASH_4_5_FRAME) {
		spawner_left = SLASH_5_CORNER_X;
		spawner_top = SLASH_5_CORNER_Y;
	}
	if(boss_phase_frame < SLASH_4_5_FRAME) {
		return;
	}
	if((boss_phase_frame < SLASH_6_FRAME) && ((boss_phase_frame % 2) == 0)) {
		slash_aimed_fire(spawner_left, spawner_top, pattern_state.speed);
		slash_spawner_step_from_4_5_to_6(spawner_left, spawner_top, 2);
	}

	#undef spawner_top
	#undef spawner_left
}

char konngara_esc_cls[] = "\x1B*";
char konngara_esc_mode_graph[] = "\x1B)3";
char konngara_esc_color_bg_black_fg_black[] = "\x1B[16;40m";
char konngara_esc_cursor_to_x0_y0_0[] = "\x1B[0;0H";
char konngara_space[] = " ";
char konngara_esc_mode_kanji[] = "\x1B)0";
char konngara_esc_color_reset[] = "\x1B[0m";
char konngara_esc_cursor_to_x0_y0_1[] = "\x1B[1;1H";
