// ReC98
// -----
// PC-98 hardware constants not covered by master.lib

#define PC98_H

/// Spaces
/// ------
/// These don't necessarily have to be relative to the top-left corner of the
/// display.

// Display-space widths, heights, and object-space coordinates
typedef int pixel_t;
typedef unsigned int upixel_t;

// VRAM widths and object-space coordinates
typedef int vram_byte_amount_t;
typedef int vram_word_amount_t;
typedef int vram_dword_amount_t;
typedef unsigned int uvram_byte_amount_t;
typedef unsigned int uvram_word_amount_t;
typedef unsigned int uvram_dword_amount_t;

// TRAM widths and object-space coordinates
typedef int tram_ank_amount_t;
typedef int tram_kanji_amount_t;
typedef unsigned int utram_kanji_amount_t;
/// ------

/// Coordinate systems
/// ------------------
/// All of these are relative to the top-left corner of the final display.
/// MODDERS: Remove the unsigned varieties.

// Display-space coordinate, with [0; RES_X[ being the visible area.
typedef int screen_x_t;
typedef unsigned int uscreen_x_t;

// Display-space coordinate, with [0; RES_Y[ being the visible area. Does not
// care about 200- or 400-line graphics modes or vertical scrolling.
typedef int screen_y_t;
typedef unsigned int uscreen_y_t;

// VRAM X coordinate, ranging from 0 to (RES_X / BYTE_DOTS).
typedef int vram_x_t;

// VRAM Y coordinate, ranging from 0 to either 400 or 200 depending on the
// current graphics mode, and with an added vertical scrolling offset.
typedef int vram_y_t;
typedef unsigned int uvram_y_t;

// Text RAM X coordinate, ranging from 0 to (RES_X / GLYPH_HALF_W).
typedef int tram_x_t;
typedef unsigned int utram_x_t;

// Text RAM Y coordinate, ranging from 0 to (RES_Y / GLYPH_H).
typedef int tram_y_t;
typedef unsigned int utram_y_t;
/// ------------------

/// Text
/// ----
#define GAIJI_W 16
#define GAIJI_TRAM_W (GAIJI_W / 8)
#define GLYPH_HALF_W 8
#define GLYPH_FULL_W 16
#define GLYPH_H 16

#define shiftjis_w(literal) \
	((sizeof(literal) - 1) * GLYPH_HALF_W)
/// ----

/// Graphics
/// --------
#define BYTE_DOTS 8
#define BYTE_MASK (BYTE_DOTS - 1)
#define RES_X 640
#define RES_Y 400
#define ROW_SIZE (RES_X / BYTE_DOTS)
#define PLANE_SIZE (ROW_SIZE * RES_Y)

#define PLANE_COUNT 4

typedef bool page_t;

#define COLOR_COUNT 16

#define COMPONENT_R 0
#define COMPONENT_G 1
#define COMPONENT_B 2
#define COMPONENT_COUNT 3

// The 16-color mode supports 4 bits per RGB component, for a total of
// 4,096 colors
typedef int8_t uint4_t;

#ifdef __cplusplus
	template <class ComponentType, int Range> union RGB {
		struct {
			ComponentType r, g, b;
		} c;
		ComponentType v[COMPONENT_COUNT];

		// Yes, we actually need this function in certain cases where code
		// generation calls for a 0 in the ComponentType.
		static ComponentType min() {
			return 0;
		}
		static ComponentType max() {
			return (Range - 1);
		}

		void set(ComponentType r, ComponentType g, ComponentType b) {
			this->c.r = r;
			this->c.g = g;
			this->c.b = b;
		}
	};

	template <class RGBType> struct Palette {
		RGBType colors[COLOR_COUNT];

		static int range() {
			return RGBType::Range;
		}

		RGBType& operator [](int col) {
			return colors[col];
		}

		const RGBType& operator [](int col) const {
			return colors[col];
		}
	};

	typedef RGB<uint4_t, 16> RGB4;
	typedef Palette<RGB4> Palette4;

	#define palette_foreach(tmp_col, tmp_comp, func) { \
		for(tmp_col = 0; tmp_col < COLOR_COUNT; tmp_col++) { \
			for(tmp_comp = 0; tmp_comp < COMPONENT_COUNT; tmp_comp++) { \
				func \
			} \
		} \
	}

	// Sets all components of all colors to the given grayscale [value].
	#define palette_set_grayscale(dst, value, tmp_col, tmp_comp) \
		palette_foreach(tmp_col, tmp_comp, { \
			dst[tmp_col].v[tmp_comp] = value; \
		})

	#define palette_copy(dst, src, tmp_col, tmp_comp) \
		palette_foreach(tmp_col, tmp_comp, { \
			dst[tmp_col].v[tmp_comp] = src[tmp_col].v[tmp_comp]; \
		})
#endif
/// --------

/// Memory segments
/// ---------------

#define SEG_TRAM_JIS 0xA000
#define SEG_TRAM_ATRB 0xA200

#define SEG_PLANE_B 0xA800
#define SEG_PLANE_R 0xB000
#define SEG_PLANE_G 0xB800
#define SEG_PLANE_E 0xE000

// Segment distance between B↔R↔G
#define SEG_PLANE_DIST_BRG 0x800

// Segment distance between G↔E
#define SEG_PLANE_DIST_E 0x2800
/// ---------------

/// EGC
/// ---
/// The PC-98 EGC always operates on 16 dots at a time.

static const int EGC_REGISTER_DOTS = 16;
static const int EGC_REGISTER_SIZE = (EGC_REGISTER_DOTS / BYTE_DOTS);
/// ---
