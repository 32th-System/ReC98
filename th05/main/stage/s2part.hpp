#define S2PARTICLE_COUNT 64

// `s2` to differentiate this structure from `boss_particle_t`, which uses the
// same sprites.
typedef struct {
	unsigned char flag;
	unsigned char angle;
	PlayfieldMotion pos;
	unsigned int zoom;
	int16_t unused[5]; // ZUN bloat
} s2particle_t;

#define s2particles (reinterpret_cast<s2particle_t *>(custom_entities))
