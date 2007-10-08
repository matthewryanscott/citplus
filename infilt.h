//---------------------------------------------------------------------------
//			Input Filtering
//
// Control+A	= Control+A
// Control+D	= Control+D
// bs			= bs
// tab			= tab
// CR			= LF
// Control+O	= 'N'
// xoff			= 'P'
// Control+Z	= Control+Z
// Esc			= Esc
// del			= bs
//
// All other Control characters mapped to Nul; all other characters
// passed unchanged.

#ifdef INPUTMAIN
extern uchar const filt_in[128] =
	{
	0,		1,		0,		0,		4,		0,		0,		0,		// 0-7
	'\b',	'\t',	0,		0,		0,		'\n',	0,		'N',	// 8-15
	0,		0,		0,		'P',	0,		0,		0,		0,		// 16-23
	0,		0,		26,		27,		0,		0,		0,		0,		// 24-31
	' ',	'!',	'"',	'#',	'$',	'%',	'&',	'\'',	// 32-39
	'(',	')',	'*',	'+',	',',	'-',	'.',	'/',	// 40-47
	'0',	'1',	'2',	'3',	'4',	'5',	'6',	'7',	// 48-55
	'8',	'9',	':',	';',	'<',	'=',	'>',	'?',	// 56-63
	'@',	'A',	'B',	'C',	'D',	'E',	'F',	'G',	// 64-71
	'H',	'I',	'J',	'K',	'L',	'M',	'N',	'O',	// 72-79
	'P',	'Q',	'R',	'S',	'T',	'U',	'V',	'W',	// 80-87
	'X',	'Y',	'Z',	'[',	'\\',	']',	'^',	'_',	// 88-95
	'`',	'a',	'b',	'c',	'd',	'e',	'f',	'g',	// 96-103
	'h',	'i',	'j',	'k',	'l',	'm',	'n',	'o',	// 104-111
	'p',	'q',	'r',	's',	't',	'u',	'v',	'w',	// 112-119
	'x',	'y',	'z',	'{',	'|',	'}',	'~',	'\b',	// 120-127
	};
#else
extern uchar const filt_in[128];
#endif
