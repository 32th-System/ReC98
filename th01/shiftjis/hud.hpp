// Pause menu
// ----------

#define PAUSE_TITLE         	"�o�`�t�r�d"
#define PAUSE_CHOICE_RESUME 	"�ĊJ"
#define PAUSE_CHOICE_QUIT   	"�I��"

#define QUIT_TITLE     	"�{���ɏI�������Ⴄ�́H"
#define QUIT_CHOICE_NO 	"�����ł���"
#define QUIT_CHOICE_YES	"�͂���"

// ZUN bloat: Does this mean that this menu used to be shown in text RAM?
#define PAUSE_CURSOR_INITIAL	"���@�@�@�@�@�@"

#define PAUSE_CURSOR        	"��"

// Leaving one fullwidth space for the cursor in front of each, and another
// one to right-pad the first option.
#define PAUSE_CHOICE_0	"�@" PAUSE_CHOICE_RESUME "�@"
#define PAUSE_CHOICE_1	"�@" PAUSE_CHOICE_QUIT
#define QUIT_CHOICE_0 	"�@" QUIT_CHOICE_NO "�@"
#define QUIT_CHOICE_1 	"�@" QUIT_CHOICE_YES
#define PAUSE_CHOICES 	PAUSE_CHOICE_0 PAUSE_CHOICE_1
#define QUIT_CHOICES  	QUIT_CHOICE_0 QUIT_CHOICE_1
// ----------
