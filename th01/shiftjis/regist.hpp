#include "th01/shiftjis/title.hpp"

#define REGIST_TITLE             	GAME_TITLE "�@���҂̋L�^"
#define REGIST_TITLE_WITH_SPACE  	REGIST_TITLE "�@"
#define REGIST_TITLE_RANKS { \
	"�@�C�[�W�[�@", \
	"�@�m�[�}���@", \
	"�@�n�[�h�@�@", \
	"���i�e�B�b�N", \
}

static const pixel_t REGIST_TITLE_W = shiftjis_w(REGIST_TITLE_WITH_SPACE);

#define REGIST_HEADER_PLACE      	"�@��@�ʁ@"
#define REGIST_HEADER_NAME       	"�@�@���@�@�O�@�@"
#define REGIST_HEADER_SCORE      	"�@�@���@�@�_�@�@"
#define REGIST_HEADER_STAGE_ROUTE	"�X�e�[�W�E���[�g"
#define REGIST_PLACE_0           	"�@�ˁ@�_�@"
#define REGIST_PLACE_1           	"����������"
#define REGIST_PLACE_2           	"�@�V�@��@"
#define REGIST_PLACE_3           	"�@�_�@��@"
#define REGIST_PLACE_4           	"�@�n�@��@"
#define REGIST_PLACE_5           	"�@�l�@��@"
#define REGIST_PLACE_6           	"�@��@���@"
#define REGIST_PLACE_7           	"�A�@�z�@�t"
#define REGIST_PLACE_8           	"�ˁ@�}�@�t"
#define REGIST_PLACE_9           	"�C�@�Ɓ@��"
#define REGIST_NAME_SPACES       	"�@�@�@�@�@�@�@�@"
#define REGIST_NAME_BLANK        	"�Q�Q�Q�Q�Q�Q�Q�Q"
#define REGIST_STAGE_ROUTE_DASH  	"�|"
#define REGIST_STAGE_MAKAI       	"���E"
#define REGIST_STAGE_JIGOKU      	"�n��"

#define ALPHABET_A    	"��"
#define ALPHABET_SPACE	"SP"
#define ALPHABET_LEFT 	"��"
#define ALPHABET_RIGHT	"��"
#define ALPHABET_ENTER	"�I"

// ZUN bloat: Storing fullwidth characters as regular 16-bit big-endian
// integers would have worked just fine.
#define kanji_to_le(kanji) ( \
	(static_cast<uint16_t>(kanji) << 8) | (static_cast<uint16_t>(kanji) >> 8) \
)

#define KANJI_A         	kanji_to_le('�`')
#define KANJI_a         	kanji_to_le('��')
#define KANJI_b         	kanji_to_le('��')
#define KANJI_0         	kanji_to_le('�O')
#define KANJI_SP        	kanji_to_le('�@')
#define KANJI_UNDERSCORE	kanji_to_le('�Q')

const uint16_t ALPHABET_SYMS[] = {
	kanji_to_le('�I'), kanji_to_le('�H'), kanji_to_le('��'), kanji_to_le('��'),
	kanji_to_le('��'), kanji_to_le('��'), kanji_to_le('��'), kanji_to_le('��'),
	kanji_to_le('��'), kanji_to_le('��'), kanji_to_le('��'), kanji_to_le('��'),
	kanji_to_le('�c'), kanji_to_le('�g'), kanji_to_le('�h'), kanji_to_le('�^'),
	kanji_to_le('�D'), kanji_to_le('�E'),
};
