// --------------------------------------------------------------------------
// Citadel: Music.CPP
//
// This module should contain all the code specific to playing music and
// sound out the PC speaker.
//
// Most of this was lifted from CenCit, only improved and debugged.

#include "ctdl.h"
#pragma hdrstop

#include "music.h"
#include "log.h"
#include "term.h"
#include "extmsg.h"


void TERMWINDOWMEMBER playSound(const char *snd)
	{
	if (cfg.music && CurrentUser->IsMusic())
		{
		TermCap->putCode("[");
		TermCap->putCode(snd);
		TermCap->putCode("");

		if (snd[0] == 'Z')
			{
			sound_effect(snd + 1);
			}
		else
			{
			play(snd + 1);
			}
		}
	}


// --------------------------------------------------------------------------
// Do the sound effect

void TERMWINDOWMEMBER sound_effect(const char *Sound_Buffer)
	{
	int i, l, pos;
	uint tone = 0, duration = 0;
	char ch, work[20];

	l = strlen(Sound_Buffer);
	i = 0;

	DebugOut(40, Sound_Buffer);

	while (i < l)
		{
		switch (toupper(Sound_Buffer[i]))
			{
			case 'F':   // Frequency
				{
				pos = 0;
				do
					{
					ch = Sound_Buffer[++i];
					work[pos++] = ch;
					} while (isdigit(ch));

				work[pos] = 0;
				tone = atoi(work);
				break;
				}

			case 'D':   // Duration
				{
				pos = 0;
				do
					{
					ch = Sound_Buffer[++i];
					work[pos++] = ch;
					} while (isdigit(ch));

				work[pos] = 0;
				duration = atoi(work);
				break;
				}

			default:
				{
				i++;
				break;
				}
			}
		}

	submit_sound(tone, duration);
	}


// --------------------------------------------------------------------------
// Play the musical score

void TERMWINDOWMEMBER play(const char *Sound_Buffer)
	{
	// -----------------------------------------------------------
	//	This frequency table contains the 8 octaves of a piano
	//	keyboard including the sharps and flats
	//
	//	NOTE:	this does not have perfect pitch because we are
	//			currently using integers.
	//	------------------------------------------------------------

		//	Octaves 	0	1		2		3		4		5		6
static int c_notes[] = {33, 66, 	131,	262,	523,	1046,	2093};
static int csnotes[] = {35, 69, 	139,	277,	554,	1108,	2217};
static int dfnotes[] = {35, 69, 	139,	277,	554,	1108,	2217};
static int d_notes[] = {37, 73, 	147,	294,	587,	1175,	2349};
static int dsnotes[] = {39, 78, 	156,	311,	622,	1245,	2489};
static int efnotes[] = {39, 78, 	156,	311,	622,	1245,	2489};
static int e_notes[] = {41, 82, 	165,	330,	659,	1329,	2637};
static int f_notes[] = {44, 87, 	175,	349,	698,	1397,	2794};
static int fsnotes[] = {46, 93, 	185,	370,	740,	1480,	2960};
static int gfnotes[] = {46, 93, 	185,	370,	740,	1480,	2960};
static int g_notes[] = {49, 98, 	196,	392,	784,	1568,	3136};
static int gsnotes[] = {52, 104,	208,	415,	831,	1661,	3322};
static int afnotes[] = {52, 104,	208,	415,	831,	1661,	3322};
static int a_notes[] = {55, 110,	220,	440,	880,	1760,	3520};
static int asnotes[] = {58, 117,	233,	466,	923,	1865,	3729};
static int bfnotes[] = {58, 117,	233,	466,	923,	1865,	3729};
static int b_notes[] = {62, 123,	247,	494,	988,	1976,	3951};

	Bool have_note;
	int this_octave;
	int tempo;
	long note_len, this_note_len;

	int i, l, pos, last_i;
	int this_note;
	char ch;
	label work;

	l = strlen(Sound_Buffer);
	have_note = FALSE;
	this_note = 0;
	this_octave = 4;
	tempo = 120;
	this_note_len = note_len = (240000l / tempo);

	i = last_i = 0;

	DebugOut(41, Sound_Buffer);

	while (i < l)
		{
		switch (toupper(Sound_Buffer[i]))
			{
			case 'T':       // Set the tempo
				{
				pos = 0;
				do
					{
					ch = Sound_Buffer[++i];
					work[pos++] = ch;
					} while (isdigit(ch));

				work[pos] = 0;
				tempo = atoi(work);

				DebugOut(42, tempo);

				break;
				}

			case 'L':       // Set the length of the notes (default 1)
				{
				pos = 0;
				do
					{
					ch = Sound_Buffer[++i];
					work[pos++] = ch;
					} while (isdigit(ch));

				work[pos] = 0;
				note_len = (240000l / tempo) / atoi(work);
				this_note_len = note_len;
				break;
				}

			case 'P':       // Set up a pause
				{
				if (have_note)
					{
					submit_note(this_note, this_note_len);
					this_note_len = note_len;
					}

				this_note = 0;
				have_note = TRUE;
				i++;
				if (isdigit(Sound_Buffer[i]))
					{
					pos = 0;
					ch = Sound_Buffer[i];
					do
						{
						work[pos++] = ch;
						ch = Sound_Buffer[++i];
						} while (isdigit(ch));

					work[pos] = 0;
					this_note_len = (240000l / tempo) / atoi(work);
					}
				break;
				}

			case 'O':       // Set up the octave
				{
				i++;
				this_octave = Sound_Buffer[i] - '0';
				i++;
				if (this_octave < 0 || this_octave > 7)
					{
					DebugOut(43, 'O');
					this_octave = 4;
					}

				break;
				}

			case 'N':
				{
				if (have_note)
					{
					submit_note(this_note, this_note_len);
					this_note_len = note_len;
					}

				have_note = TRUE;
				i++;
				if (isdigit(Sound_Buffer[i]))
					{
					int note;

					pos = 0;
					ch = Sound_Buffer[i];
					do
						{
						work[pos++] = ch;
						ch = Sound_Buffer[++i];
						} while (isdigit(ch));

					work[pos] = 0;
					note = atoi(work);

					if (note == 0)
						{
						this_note = 0;
						}
					else if (note < 85)
						{
						int *base;

						switch (note % 12)
							{
							case 0: 	base = c_notes; break;
							case 1: 	base = csnotes; break;
							case 2: 	base = d_notes; break;
							case 3: 	base = dsnotes; break;
							case 4: 	base = e_notes; break;
							case 5: 	base = f_notes; break;
							case 6: 	base = fsnotes; break;
							case 7: 	base = g_notes; break;
							case 8: 	base = gsnotes; break;
							case 9: 	base = a_notes; break;
							case 10:	base = asnotes; break;
							case 11:	base = b_notes; break;
							}

						this_note = base[note / 12];
						}
					else
						{
						DebugOut(44, note);
						have_note = FALSE;
						}
					}
				}

			case '<':       // drop our octave for the next note
				{
				this_octave--;
				i++;
				if (this_octave < 0)
					{
					DebugOut(43, '<');
					this_octave = 0;
					}
				break;
				}

			case '>':       // raise our octave for the next note
				{
				this_octave++;
				i++;
				if (this_octave > 7)
					{
					DebugOut(43, '>');
					this_octave = 7;
					}
				break;
				}

			case '.':       // increase this note's playing time by 3/2
				{
				this_note_len = (this_note_len * 3) / 2;
				i++;
				break;
				}

			case 'A':
				{
				if (have_note)
					{
					submit_note(this_note, this_note_len);
					this_note_len = note_len;
					}

				i++;
				if (Sound_Buffer[i] == '#' || Sound_Buffer[i] == '+')
					{
					this_note = asnotes[this_octave];
					have_note = TRUE;
					i++;
					}
				else if (Sound_Buffer[i] == '-')
					{
					this_note = afnotes[this_octave];
					have_note = TRUE;
					i++;
					}
				else
					{
					this_note = a_notes[this_octave];
					have_note = TRUE;
					}

				if (isdigit(Sound_Buffer[i]))
					{
					pos = 0;
					ch = Sound_Buffer[i];
					do
						{
						work[pos++] = ch;
						ch = Sound_Buffer[++i];
						} while (isdigit(ch));

					work[pos] = 0;
					this_note_len = (240000l / tempo) / atoi(work);
					}

				break;
				}

			case 'B':
				{
				if (have_note)
					{
					submit_note(this_note, this_note_len);
					this_note_len = note_len;
					}

				i++;
				if (Sound_Buffer[i] == '-')
					{
					this_note = bfnotes[this_octave];
					have_note = TRUE;
					i++;
					}
				else
					{
					this_note = b_notes[this_octave];
					have_note = TRUE;
					}

				if (isdigit(Sound_Buffer[i]))
					{
					pos = 0;
					ch = Sound_Buffer[i];
					do
						{
						work[pos++] = ch;
						ch = Sound_Buffer[++i];
						} while (isdigit(ch));

					work[pos] = 0;
					this_note_len = (240000l / tempo) / atoi(work);
					}

				break;
				}

			case 'C':
				{
				if (have_note)
					{
					submit_note(this_note, this_note_len);
					this_note_len = note_len;
					}

				i++;
				if (Sound_Buffer[i] == '#' || Sound_Buffer[i] == '+')
					{
					this_note = csnotes[this_octave];
					have_note = TRUE;
					i++;
					}
				else
					{
					this_note = c_notes[this_octave];
					have_note = TRUE;
					}

				if (isdigit(Sound_Buffer[i]))
					{
					pos = 0;
					ch = Sound_Buffer[i];
					do
						{
						work[pos++] = ch;
						ch = Sound_Buffer[++i];
						} while (isdigit(ch));

					work[pos] = 0;
					this_note_len = (240000l / tempo) / atoi(work);
					}

				break;
				}

			case 'D':
				{
				if (have_note)
					{
					submit_note(this_note, this_note_len);
					this_note_len = note_len;
					}

				i++;
				if (Sound_Buffer[i] == '#' || Sound_Buffer[i] == '+')
					{
					this_note = dsnotes[this_octave];
					have_note = TRUE;
					i++;
					}
				else if (Sound_Buffer[i] == '-')
					{
					this_note = dfnotes[this_octave];
					have_note = TRUE;
					i++;
					}
				else
					{
					this_note = d_notes[this_octave];
					have_note = TRUE;
					}

				if (isdigit(Sound_Buffer[i]))
					{
					pos = 0;
					ch = Sound_Buffer[i];
					do
						{
						work[pos++] = ch;
						ch = Sound_Buffer[++i];
						} while (isdigit(ch));

					work[pos] = 0;
					this_note_len = (240000l / tempo) / atoi(work);
					}

				break;
				}

			case 'E':
				{
				if (have_note)
					{
					submit_note(this_note, this_note_len);
					this_note_len = note_len;
					}

				i++;
				if (Sound_Buffer[i] == '-')
					{
					this_note = efnotes[this_octave];
					have_note = TRUE;
					i++;
					}
				else
					{
					this_note = e_notes[this_octave];
					have_note = TRUE;
					}

				if (isdigit(Sound_Buffer[i]))
					{
					pos = 0;
					ch = Sound_Buffer[i];
					do
						{
						work[pos++] = ch;
						ch = Sound_Buffer[++i];
						} while (isdigit(ch));

					work[pos] = 0;
					this_note_len = (240000l / tempo) / atoi(work);
					}

				break;
				}

			case 'F':
				{
				if (have_note)
					{
					submit_note(this_note,this_note_len);
					this_note_len = note_len;
					}

				i++;
				if (Sound_Buffer[i] == '#' || Sound_Buffer[i] == '+')
					{
					this_note = fsnotes[this_octave];
					have_note = TRUE;
					i++;
					}
				else
					{
					this_note = f_notes[this_octave];
					have_note = TRUE;
					}

				if (isdigit(Sound_Buffer[i]))
					{
					pos = 0;
					ch = Sound_Buffer[i];
					do
						{
						work[pos++] = ch;
						ch = Sound_Buffer[++i];
						} while (isdigit(ch));

					work[pos] = 0;
					this_note_len = (240000l / tempo) / atoi(work);
					}

				break;
				}

			case 'G':
				{
				if (have_note)
					{
					submit_note(this_note,this_note_len);
					this_note_len = note_len;
					}

				i++;
				if (Sound_Buffer[i] == '#' || Sound_Buffer[i] == '+')
					{
					this_note = gsnotes[this_octave];
					have_note = TRUE;
					i++;
					}
				else if (Sound_Buffer[i] == '-')
					{
					this_note = gfnotes[this_octave];
					have_note = TRUE;
					i++;
					}
				else
					{
					this_note = g_notes[this_octave];
					have_note = TRUE;
					}

				if (isdigit(Sound_Buffer[i]))
					{
					pos = 0;
					ch = Sound_Buffer[i];
					do
						{
						work[pos++] = ch;
						ch = Sound_Buffer[++i];
						} while (isdigit(ch));

					work[pos] = 0;
					this_note_len = (240000l / tempo) / atoi(work);
					}

				break;
				}

			case 'M':       // Music style - next char is FBNL or S
				{
				i += 2;
				break;
				}

			case 14:
				{
				i = l;
				break;
				}

			default:		// ignore unknowns, like ' ' or whatever
				{
				ch = Sound_Buffer[i];

				if (ch != ' ')
					{
					DebugOut(45, ch, i);
					}

				i++;
				break;
				}
			}

		if (i == last_i)
			{
			i++;
			DebugOut(45, ch, i);
			}

		last_i = i;
		}

	if (have_note)
		{
		submit_note(this_note, this_note_len);
		this_note_len = note_len;
		}

	submit_sound(0, 0);
	}

void TERMWINDOWMEMBER submit_note(int note, long note_len)
	{
	int duration, pause;

	pause = (int) (note_len / 8);
	duration = (int) ((note_len * 7) / 8);

	DebugOut(46, note, duration, note_len);

	submit_sound(note, duration);
	submit_sound(0, pause);
	}

void init_internal_sound(void)
	{
	}


// --------------------------------------------
// Call this routine before returning to DOS...

void deinit_internal_sound(void)
	{
#ifndef WINCIT
	nosound();
#endif
	}


// ---------------------------------------------------------
// This routine is used to submit sounds to the sound queue.

void TERMWINDOWMEMBER submit_sound(int freq, int dly)
	{
#ifndef WINCIT
	sound(freq);
	delay(dly);
	nosound();
#endif
	}
