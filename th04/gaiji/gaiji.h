// ReC98
// -----
// Gaiji available to TH04

#include "th02/gaiji/boldfont.h"
#include "th04/gaiji/bar.h"

typedef enum {
	g_EMPTY = 0x02,
	gaiji_bar(0x20),
	gaiji_bar_max(0x30),
	gaiji_boldfont(0xA0),
	gs_DOT = 0xC4,
	gs_BOMB = 0xD3, // ◉? ⦿? 🎯? 🖸? Or simply 💣?
	gs_YINYANG, // ☯
	gs_TEN = 0xE6, // 点
	gs_YUME, // 夢
	gs_TAMA, // 弾
	gs_ALL, // "All"
	g_HISCORE_STAGE_EMPTY = 0xEF,
	g_NONE = 0xFF,
} gaiji_th04_t;
