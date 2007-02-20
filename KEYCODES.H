// --------------------------------------------------------------------------
// Citadel: Keycodes.H
//
// #defines for all special extended keys used by Citadel.


#ifndef WINCIT

#define ALT_A			30					// Accounting toggle
#define ALT_B			48					// Psycho toggle
#define ALT_C			46					// Chat mode
#define ALT_D			32					// Debug mode
#define ALT_E			18					// Event timeout
#define ALT_F			33					// Force event
#define ALT_G			34
#define ALT_H			35
#define ALT_I			23
#define ALT_J			36
#define ALT_K			37					// Scroll back
#define ALT_L			38					// Lock console
#define ALT_M			50
#define ALT_N			49					// Next task.
#define ALT_O			24					// Open new task.
#define ALT_P			25					// Print toggle
#define ALT_Q			16
#define ALT_R			19					// Repeat event
#define ALT_S			31					// Screen saver
#define ALT_T			20					// Twit toggle
#define ALT_U			22					// Ignore uptime
#define ALT_V			47					// Verified toggle
#define ALT_W			17
#define ALT_X			45					// Exit Citadel
#define ALT_Y			21
#define ALT_Z			44					// Timeout
#define ALT_1			120
#define ALT_2			121
#define ALT_3			122
#define ALT_4			123
#define ALT_5			124
#define ALT_6			125
#define ALT_7			126
#define ALT_8			127
#define ALT_9			128
#define ALT_0			129
#define ALT_MIN			130
#define ALT_EQU			131

#define SFT_TAB			15

#define F1				59					// Shut down
#define F2				60					// Reset
#define F3				61					// Request
#define F4				62					// Status screen
#define F5				63					// Console/Modem toggle
#define F6				64					// Sysop Menu
#define F7				65					// Bells toggle
#define F8				66					// Chat mode
#define F9				67					// Chat toggle
#define F10				68					// Help toggle
#define F11				133
#define F12				134

#define ALT_F1			104
#define ALT_F2			105
#define ALT_F3			106
#define ALT_F4			107
#define ALT_F5			108
#define ALT_F6			109					// Aide
#define ALT_F7			110
#define ALT_F8			111
#define ALT_F9			112
#define ALT_F10			113
#define ALT_F11			139
#define ALT_F12			140

#define SFT_F1			84
#define SFT_F2			85
#define SFT_F3			86
#define SFT_F4			87
#define SFT_F5			88
#define SFT_F6			89					// Sysop
#define SFT_F7			90
#define SFT_F8			91
#define SFT_F9			92
#define SFT_F10			93
#define SFT_F11			135
#define SFT_F12			136

#define CTL_F1			94					// speech
#define CTL_F2			95
#define CTL_F3			96
#define CTL_F4			97
#define CTL_F5			98
#define CTL_F6			99					// Sysop windows
#define CTL_F7			100
#define CTL_F8			101
#define CTL_F9			102
#define CTL_F10			103
#define CTL_F11			137
#define CTL_F12			138

#define PGUP			73					// More time
#define PGDN			81					// Less time

#define CURS_HOME		71
#define CURS_UP			72
#define CURS_LEFT		75
#define CURS_RIGHT		77
#define CURS_END		79
#define CURS_DOWN		80

#define CURS_INS		82
#define CURS_DEL		83

#define CTL_PGUP		132
#define CTL_PGDN		118

#define CTL_CURS_HOME	119
#define CTL_CURS_LEFT	115
#define CTL_CURS_RIGHT	116
#define CTL_CURS_END	117

#else

// WINCIT: VK_stuff | our own Alt, Ctrl, Shift stuff.
// We leave the lower 16 bits free: Windows gives us 16-bit values for
// VK_s (even though it seems to only set the lower 8 of them). We also don't
// use the upper 8 bits: this value is shifted left by 8 bits and returned as
// a 32-bit integer to the rest of Citadel. This is done because if the
// character is a simple character, it is returned in the lower 8 bits.

#define WC_ALT		0x010000
#define WC_CTRL		0x020000
#define	WC_SHIFT	0x030000

#define ALT_A			(WC_ALT | 'A')		// Accounting toggle
#define ALT_B			(WC_ALT | 'B')		// Psycho toggle
#define ALT_C			(WC_ALT | 'C')		// Chat mode
#define ALT_D			(WC_ALT | 'D')		// Debug mode
#define ALT_E			(WC_ALT | 'E')		// Event timeout
#define ALT_F			(WC_ALT | 'F')		// Force event
#define ALT_G			(WC_ALT | 'G')
#define ALT_H			(WC_ALT | 'H')
#define ALT_I			(WC_ALT | 'I')
#define ALT_J			(WC_ALT | 'J')
#define ALT_K			(WC_ALT | 'K')		// Scroll back
#define ALT_L			(WC_ALT | 'L')		// Lock console
#define ALT_M			(WC_ALT | 'M')
#define ALT_N			(WC_ALT | 'N')		// Next task.
#define ALT_O			(WC_ALT | 'O')		// Open new task.
#define ALT_P			(WC_ALT | 'P')		// Print toggle
#define ALT_Q			(WC_ALT | 'Q')
#define ALT_R			(WC_ALT | 'R')		// Repeat event
#define ALT_S			(WC_ALT | 'S')		// Screen saver
#define ALT_T			(WC_ALT | 'T')		// Twit toggle
#define ALT_U			(WC_ALT | 'U')		// Ignore uptime
#define ALT_V			(WC_ALT | 'V')		// Verified toggle
#define ALT_W			(WC_ALT | 'W')
#define ALT_X			(WC_ALT | 'X')		// Exit Citadel
#define ALT_Y			(WC_ALT | 'Y')
#define ALT_Z			(WC_ALT | 'Z')		// Timeout
#define ALT_1			(WC_ALT | '1')
#define ALT_2			(WC_ALT | '2')
#define ALT_3			(WC_ALT | '3')
#define ALT_4			(WC_ALT | '4')
#define ALT_5			(WC_ALT | '5')
#define ALT_6			(WC_ALT | '6')
#define ALT_7			(WC_ALT | '7')
#define ALT_8			(WC_ALT | '8')
#define ALT_9			(WC_ALT | '9')
#define ALT_0			(WC_ALT | '0')

#define ALT_MIN			(WC_ALT | 189)		// These codes (189 and 187)
#define ALT_EQU			(WC_ALT | 187)		// don't seem to be in WINDOWS.H
											// But they are what Windows NT
											// 3.51 are giving me.

#define SFT_TAB			(WC_SHIFT | VK_TAB)

#define F1				VK_F1				// Shut down
#define F2				VK_F2				// Reset
#define F3				VK_F3				// Request
#define F4				VK_F4				// Status screen
#define F5				VK_F5				// Console/Modem toggle
#define F6				VK_F6				// Sysop Menu
#define F7				VK_F7				// Bells toggle
#define F8				VK_F8				// Chat mode
#define F9				VK_F9				// Chat toggle
#define F10				VK_F10				// Help toggle
#define F11				VK_F11
#define F12				VK_F12

#define ALT_F1			(WC_ALT | VK_F1)
#define ALT_F2			(WC_ALT | VK_F2)
#define ALT_F3			(WC_ALT | VK_F3)
#define ALT_F4			(WC_ALT | VK_F4)
#define ALT_F5			(WC_ALT | VK_F5)
#define ALT_F6			(WC_ALT | VK_F6)	// Aide
#define ALT_F7			(WC_ALT | VK_F7)
#define ALT_F8			(WC_ALT | VK_F8)
#define ALT_F9			(WC_ALT | VK_F9)
#define ALT_F10			(WC_ALT | VK_F10)
#define ALT_F11			(WC_ALT | VK_F11)
#define ALT_F12			(WC_ALT | VK_F12)

#define SFT_F1			(WC_SHIFT | VK_F1)
#define SFT_F2			(WC_SHIFT | VK_F2)
#define SFT_F3			(WC_SHIFT | VK_F3)
#define SFT_F4			(WC_SHIFT | VK_F4)
#define SFT_F5			(WC_SHIFT | VK_F5)
#define SFT_F6			(WC_SHIFT | VK_F6)	// Sysop
#define SFT_F7			(WC_SHIFT | VK_F7)
#define SFT_F8			(WC_SHIFT | VK_F8)
#define SFT_F9			(WC_SHIFT | VK_F9)
#define SFT_F10			(WC_SHIFT | VK_F10)
#define SFT_F11			(WC_SHIFT | VK_F11)
#define SFT_F12			(WC_SHIFT | VK_F12)

#define CTL_F1			(WC_CTRL | VK_F1)	// speech
#define CTL_F2			(WC_CTRL | VK_F2)
#define CTL_F3			(WC_CTRL | VK_F3)
#define CTL_F4			(WC_CTRL | VK_F4)
#define CTL_F5			(WC_CTRL | VK_F5)
#define CTL_F6			(WC_CTRL | VK_F6)	// Sysop windows
#define CTL_F7			(WC_CTRL | VK_F7)
#define CTL_F8			(WC_CTRL | VK_F8)
#define CTL_F9			(WC_CTRL | VK_F9)
#define CTL_F10			(WC_CTRL | VK_F10)
#define CTL_F11			(WC_CTRL | VK_F11)
#define CTL_F12			(WC_CTRL | VK_F12)

#define PGUP			VK_PRIOR			// More time
#define PGDN			VK_NEXT				// Less time

#define CURS_HOME		VK_HOME
#define CURS_UP			VK_UP
#define CURS_LEFT		VK_LEFT
#define CURS_RIGHT		VK_RIGHT
#define CURS_END		VK_END
#define CURS_DOWN		VK_DOWN

#define CURS_INS		VK_INSERT
#define CURS_DEL		VK_DELETE

#define CTL_PGUP		(WC_CTRL | VK_PRIOR)
#define CTL_PGDN		(WC_CTRL | VK_NEXT)

#define CTL_CURS_HOME	(WC_CTRL | VK_HOME)
#define CTL_CURS_LEFT	(WC_CTRL | VK_LEFT)
#define CTL_CURS_RIGHT	(WC_CTRL | VK_RIGHT)
#define CTL_CURS_END	(WC_CTRL | VK_DOWN)

#endif
