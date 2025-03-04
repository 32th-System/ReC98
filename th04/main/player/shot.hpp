// Sets [velocity] to a vector with the given [angle] and a 12-pixel length.
//
// TH05 also insists on setting shot_t::angle via a ridiculous out-of-bounds
// access, and therefore *must* be called with [velocity] pointing inside a
// shot_t structure!
SPPoint pascal near shot_velocity_set(
	SPPoint near* velocity, unsigned char angle
);

struct Shot {
	char flag;
	char age;
	PlayfieldMotion pos;
	// The displayed sprite changes between this one and
	// [patnum_base + 1] every two frames.
#if GAME == 5
	char patnum_base;
	char type;
#else
	int patnum_base;
#endif
	char damage;
	char angle; // Unused in TH04

	void from_option_l(float offset = 0.0f) {
		this->pos.cur.x -= PLAYER_OPTION_DISTANCE + offset;
	}

	void from_option_r(float offset = 0.0f) {
		this->pos.cur.x += PLAYER_OPTION_DISTANCE + offset;
	}

	#if (GAME == 5)
		void set_option_sprite() {
			this->patnum_base = PAT_SHOT_SUB;
		}

		void set_option_sprite_and_damage(char damage) {
			set_option_sprite();
			this->damage = damage;
		}
	#endif

	void set_random_angle_forwards(
		unsigned char min = -0x48, unsigned char max = -0x38
	) {
		shot_velocity_set(
			(SPPoint near*)&this->pos.velocity,
			randring1_next8_and_ge_lt(min, max)
		);
	}
};

#if (GAME == 5)
	#define SHOT_COUNT 64
#else
	#define SHOT_COUNT 68
#endif

extern unsigned char shot_time;
extern Shot near shots[SHOT_COUNT];

// Points to the next free entry in [shots].
extern Shot near *shot_ptr;
// Index of the last valid entry in [shots].
extern char shot_last_id;

// Current area on the playfield for which to check shot collisions.
extern SPPoint shot_hitbox_center;
extern SPPoint shot_hitbox_radius;

// If `true`, shot damage is divided by 4 during bombs. In TH05, `true` also
// slightly increases damage output for Reimu's and Yuuka's shots, and is also
// used against midbosses.
extern bool shots_hittest_against_boss;

// Only used for hit detection in TH04. TH05 also uses it in shots_render().
struct shot_alive_t {
	// For TH05, [x] == Subpixel::None() indicates that this shot has been
	// converted to a hitshot.
	SPPoint pos;

	Shot near *shot;
};
extern unsigned int shots_alive_count;
extern shot_alive_t shots_alive[SHOT_COUNT];

// Searches and returns the next free shot slot, or a nullptr if there are no
// more free ones.
Shot near* near shots_add(void);

// Processes collisions of all shots against the shot_hitbox, decays any
// colliding shots, and returns the total amount of damage dealt.
int shots_hittest(void);

inline int shots_hittest(
	const PlayfieldPoint &center,
	const subpixel_t &radius_x,
	const subpixel_t &radius_y
) {
	shot_hitbox_radius.x.v = radius_x;
	shot_hitbox_radius.y.v = radius_y;
	if(GAME == 5) {
		shot_hitbox_center = center;
	} else {
		shot_hitbox_center.x.v = center.x.v;
		shot_hitbox_center.y.v = center.y.v;
	}
	return shots_hittest();
}

// Also renders hitshots in TH05.
void near shots_render(void);

// Option laser
// ------------
// Unused in TH05, but still present in the code.

static const pixel_t SHOT_LASER_W = 8;

typedef enum {
	SHOT_LASER_CEL_0,
	SHOT_LASER_CEL_1,
	SHOT_LASER_CEL_2,
	SHOT_LASER_CEL_3,
	SHOT_LASER_CEL_4,
	SHOT_LASER_CELS,
} shot_laser_cel_t;

// Takes a Subpixel for [h].
#define shot_laser_put(left, top, h, cel) \
	_SI = h; \
	shot_laser_put_raw(left, top, cel);
void __fastcall near shot_laser_put_raw(
	screen_x_t left, vram_y_t top, shot_laser_cel_t cel
);
// ------------
