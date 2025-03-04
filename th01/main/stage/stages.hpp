extern bool16 stage_cleared;

// Amount of time until the player has to HARRY UP.
extern unsigned int stage_timer;

// Default filenames for the background image and music. Adjusted by
// scene_init_and_load().
extern char default_grp_fn[15];
extern char default_bgm_fn[15];

#define stage_is_boss(stage_id) ( \
	((stage_id % STAGES_PER_SCENE) == BOSS_STAGE) \
)
#define stage_on_route(stage_id) ( \
	stage_id >= (1 * STAGES_PER_SCENE) \
)

#define stage_resets_game_state(stage_id) ( \
	((stage_id % STAGES_PER_SCENE) == 0) || stage_is_boss(stage_id) \
)

// Render the initial stage screen and animations, depending on whether the
// current stage is the [first_stage_in_scene]. If this is the case, this
// function must be called with VRAM page 0 as the accessed page.
void stage_entrance(int stage_id, const char* bg_fn, bool16 clear_vram_page_0);

// Loads the contents of STAGE[id].DAT, and sets [default_grp_fn] and
// [default_bgm_fn] accordingly. [id] must be ≥0 and ≤9.
// (Defined in stageobj.cpp.)
void scene_init_and_load(unsigned char id);
