#include "defconv.h"

// Font weights
// ------------

// As stored in font ROM
#define WEIGHT_NORMAL 0

// Naively adds one pixel of boldness, to the left
#define WEIGHT_HEAVY 1

// Adds one pixel of boldness to the left, but preserves holes inbetween
// strokes.
#define WEIGHT_BOLD 2

// Adding another pixel of boldness, to the left, on top of WEIGHT_BOLD.
// (Very thicc!)
#define WEIGHT_BLACK 3

#define WEIGHT_COUNT 4
// ------------

#if (GAME == 1)
	// TH01-exclusive effects
	// ----------------------
	// Puts a black background behind the text. Useful if the text is rendered
	// onto the back page and should then be 2✕ scaled onto the front page.
	#define FX_CLEAR_BG 	0x200

	#define FX_UNDERLINE 	0x400
	#define FX_REVERSE  	0x800
	// ----------------------
#endif

#if (GAME <= 3)
	#define FX_WEIGHT_NORMAL (WEIGHT_NORMAL << 4)
	#define FX_WEIGHT_HEAVY  (WEIGHT_HEAVY << 4)
	#define FX_WEIGHT_BOLD   (WEIGHT_BOLD << 4)
	#define FX_WEIGHT_BLACK  (WEIGHT_BLACK << 4)

	#define FX_SPACING(spacing) \
		(spacing & 7) << 6)

	// Puts the given [str] onto the graphics RAM at the given position,
	// with the given graphics color and effect.
	void DEFCONV graph_putsa_fx(
		screen_x_t left, vram_y_t top, int16_t col_and_fx, const shiftjis_t *str
	);
#endif

#if (GAME == 1)
	// Variadic version of graph_putsa_fx().
	void graph_printf_fx(
		screen_x_t left,
		vram_y_t top,
		int16_t col_and_fx,
		const shiftjis_t *fmt,
		...
	);

	// Calculates the width of [str], displayed with the given [fx].
	int text_extent_fx(int fx, const shiftjis_t *str);
#endif
