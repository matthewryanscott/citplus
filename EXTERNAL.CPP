// --------------------------------------------------------------------------
// Citadel: External.CPP
//
// Code for external hardware drivers

#include "ctdl.h"
#pragma hdrstop

#include "ser.h"
#include "cfgfiles.h"
#include "timer.h"
#include "config.h"
#include "extmsg.h"


// --------------------------------------------------------------------------
// Contents
//
// initExtDrivers	Load up data drivers and initialize them

void initExtDrivers(void)
	{
#ifndef WINCIT
	char to[20], wo[80], *words[255];
	FILE *co;
	int po;

	if ((co = fopen(citfiles[C_HARDWARE_CIT], FO_R)) != NULL)
		{
		while (fgets(wo, 80, co) != NULL)
			{
			if (wo[0] != '#')
				{
				continue;
				}

			parse_it(words, wo);

			if (SameString(getcfgmsg(220), words[0]))
				{
				// do com driver here
				if (comDriver)
					{
					printf(getcfgmsg(221), getcfgmsg(220), bn);
					}
				else
					{
					strcpy(to, words[1]);
					strcat(to, getcfgmsg(226));
					if ((po = open(to, O_RDONLY | O_BINARY)) > 0)
						{
						long fl = filelength(po);

						comDriver = new char[(uint) fl];

						if (comDriver)
							{
							void *ptr1;
							int (cdecl *ptr2)(void);

							read(po, cdDesc, LABELSIZE);

							lseek(po, FP_OFF(comDriver), SEEK_SET);
							read(po, comDriver, (uint) (fl - FP_OFF(comDriver)));

							ptr1 = comDriver;
							ptr2 = (int (cdecl *)(void)) comDriver;

							// init_port
							FP_OFF(ptr1) = 32;
							FP_OFF(ptr2) = *(int *) ptr1;
							if (FP_OFF(ptr2))
								{
								initrs = (void (cdecl *)(int,int,int,int,int,int)) ptr2;
								}

							// close_port
							FP_OFF(ptr1) += sizeof(int);
							FP_OFF(ptr2) = *(int *) ptr1;
							if (FP_OFF(ptr2))
								{
								deinitrs = (void (cdecl *)(void)) ptr2;
								}

							// ring_detect
							FP_OFF(ptr1) += sizeof(int);
							FP_OFF(ptr2) = *(int *) ptr1;
							if (FP_OFF(ptr2))
								{
								ringstatrs = ptr2;
								}

							// have_carrier
							FP_OFF(ptr1) += sizeof(int);
							FP_OFF(ptr2) = *(int *) ptr1;
							if (FP_OFF(ptr2))
								{
								carrstatrs = ptr2;
								}

							// mi_ready
							FP_OFF(ptr1) += sizeof(int);
							FP_OFF(ptr2) = *(int *) ptr1;
							if (FP_OFF(ptr2))
								{
								statrs = ptr2;
								}

							// ib_flush
							FP_OFF(ptr1) += sizeof(int);
							FP_OFF(ptr2) = *(int *) ptr1;
							if (FP_OFF(ptr2))
								{
								flushrs = (void (cdecl *)(void)) ptr2;
								}

							// get_input
							FP_OFF(ptr1) += sizeof(int);
							FP_OFF(ptr2) = *(int *) ptr1;
							if (FP_OFF(ptr2))
								{
								getrs = ptr2;
								}

							// put_output
							FP_OFF(ptr1) += sizeof(int);
							FP_OFF(ptr2) = *(int *) ptr1;
							if (FP_OFF(ptr2))
								{
								putrs = (void (cdecl *)(char)) ptr2;
								}

							// set_dtr
							FP_OFF(ptr1) += sizeof(int);
							FP_OFF(ptr2) = *(int *) ptr1;
							if (FP_OFF(ptr2))
								{
								dtrrs = (void (cdecl *)(int)) ptr2;
								}

							// ob_flush
							FP_OFF(ptr1) += sizeof(int);
							FP_OFF(ptr2) = *(int *) ptr1;
							if (FP_OFF(ptr2))
								{
								flushoutrs = (void (cdecl *)(void)) ptr2;
								}

							close(po);
							}
						else
							{
							close(po);
							printf(getcfgmsg(222), getcfgmsg(220));
							DeinitializeTimer();
							critical(FALSE);
							exit(200);
							}
						}
					}
				}
			else if (SameString(getcfgmsg(223), words[0]))
				{
				// do vid driver here
				if (vidDriver)
					{
					printf(getcfgmsg(221), getcfgmsg(223), bn);
					}
				else
					{
					strcpy(to, words[1]);
					strcat(to, getcfgmsg(227));
					if ((po = open(to, O_RDONLY | O_BINARY)) > 0)
						{
						long fl = filelength(po);

						vidDriver = new char[(uint) fl];

						if (vidDriver)
							{
							read(po, vdDesc, LABELSIZE);

							lseek(po, FP_OFF(vidDriver), SEEK_SET);
							read(po, vidDriver, (uint) (fl - FP_OFF(vidDriver)));
							close(po);
							}
						else
							{
							close(po);
							printf(getcfgmsg(222), getcfgmsg(223));
							DeinitializeTimer();
							critical(FALSE);
							exit(200);
							}
						}
					}
				}
			else if (SameString(getcfgmsg(224), words[0]))
				{
				// do kbd driver here
				if (kbdDriver)
					{
					printf(getcfgmsg(221), getcfgmsg(224), bn);
					}
				else
					{
					strcpy(to, words[1]);
					strcat(to, getcfgmsg(228));
					if ((po = open(to, O_RDONLY | O_BINARY)) > 0)
						{
						long fl = filelength(po);

						kbdDriver = new char[(uint) fl];

						if (kbdDriver)
							{
							void *ptr1;
							int (cdecl *ptr2)(void);

							read(po, kdDesc, LABELSIZE);

							lseek(po, FP_OFF(kbdDriver), SEEK_SET);
							read(po, kbdDriver, (uint) (fl - FP_OFF(kbdDriver)));

							ptr1 = kbdDriver;
							ptr2 = (int (cdecl *)(void)) kbdDriver;

							// init_kbd
							FP_OFF(ptr1) = 32;
							FP_OFF(ptr2) = *(int *) ptr1;
							if (FP_OFF(ptr2))
								{
								initkbd = (void (cdecl *)(void)) ptr2;
								}

							// close_kbd
							FP_OFF(ptr1) += sizeof(int);
							FP_OFF(ptr2) = *(int *) ptr1;
							if (FP_OFF(ptr2))
								{
								deinitkbd = (void (cdecl *)(void)) ptr2;
								}

							// statcon
							FP_OFF(ptr1) += sizeof(int);
							FP_OFF(ptr2) = *(int *) ptr1;
							if (FP_OFF(ptr2))
								{
								statcon = ptr2;
								}

							// getcon
							FP_OFF(ptr1) += sizeof(int);
							FP_OFF(ptr2) = *(int *) ptr1;
							if (FP_OFF(ptr2))
								{
								getcon = ptr2;
								}

							// sp_press
							FP_OFF(ptr1) += sizeof(int);
							FP_OFF(ptr2) = *(int *) ptr1;
							if (FP_OFF(ptr2))
								{
								sp_press = (Bool (cdecl *)(void)) ptr2;
								}

							close(po);

							(*initkbd)();
							}
						else
							{
							close(po);
							printf(getcfgmsg(222), getcfgmsg(224));
							DeinitializeTimer();
							critical(FALSE);
							exit(200);
							}
						}
					}
				}
			else if (SameString(getcfgmsg(225), words[0]))
				{
				// do snd driver here
				if (sndDriver)
					{
					printf(getcfgmsg(221), getcfgmsg(225), bn);
					}
				else
					{
					strcpy(to, words[1]);
					strcat(to, getcfgmsg(229));
					if ((po = open(to, O_RDONLY | O_BINARY)) > 0)
						{
						long fl = filelength(po);

						sndDriver = new char[(uint) fl];

						if (sndDriver)
							{
							void *ptr1;
							int (cdecl *ptr2)(void);

							read(po, sdDesc, LABELSIZE);

							lseek(po, FP_OFF(sndDriver), SEEK_SET);
							read(po, sndDriver, (uint) (fl - FP_OFF(sndDriver)));

							ptr1 = sndDriver;
							ptr2 = (int (cdecl *)(void)) sndDriver;

							// init_sound
							FP_OFF(ptr1) = 32;
							FP_OFF(ptr2) = *(int *) ptr1;
							if (FP_OFF(ptr2))
								{
								init_sound = ptr2;
								}

							// close_sound
							FP_OFF(ptr1) += sizeof(int);
							FP_OFF(ptr2) = *(int *) ptr1;
							if (FP_OFF(ptr2))
								{
								close_sound = ptr2;
								}

							// get_version
							FP_OFF(ptr1) += sizeof(int);
							FP_OFF(ptr2) = *(int *) ptr1;
							if (FP_OFF(ptr2))
								{
								get_version = ptr2;
								}

							// query_drivers
							FP_OFF(ptr1) += sizeof(int);
							FP_OFF(ptr2) = *(int *) ptr1;
							if (FP_OFF(ptr2))
								{
								query_drivers = ptr2;
								}

							// query_status
							FP_OFF(ptr1) += sizeof(int);
							FP_OFF(ptr2) = *(int *) ptr1;
							if (FP_OFF(ptr2))
								{
								query_status = ptr2;
								}

							// start_snd_src
							FP_OFF(ptr1) += sizeof(int);
							FP_OFF(ptr2) = *(int *) ptr1;
							if (FP_OFF(ptr2))
								{
								start_snd_src = (int (cdecl *)(int, const void *)) ptr2;
								}

							// play_sound
							FP_OFF(ptr1) += sizeof(int);
							FP_OFF(ptr2) = *(int *) ptr1;
							if (FP_OFF(ptr2))
								{
								play_sound = (int (cdecl *)(int)) ptr2;
								}

							// stop_sound
							FP_OFF(ptr1) += sizeof(int);
							FP_OFF(ptr2) = *(int *) ptr1;
							if (FP_OFF(ptr2))
								{
								stop_sound = (int (cdecl *)(int)) ptr2;
								}

							// pause_sound
							FP_OFF(ptr1) += sizeof(int);
							FP_OFF(ptr2) = *(int *) ptr1;
							if (FP_OFF(ptr2))
								{
								pause_sound = (int (cdecl *)(int)) ptr2;
								}

							// resume_sound
							FP_OFF(ptr1) += sizeof(int);
							FP_OFF(ptr2) = *(int *) ptr1;
							if (FP_OFF(ptr2))
								{
								resume_sound = (int (cdecl *)(int)) ptr2;
								}

							// read_snd_stat
							FP_OFF(ptr1) += sizeof(int);
							FP_OFF(ptr2) = *(int *) ptr1;
							if (FP_OFF(ptr2))
								{
								read_snd_stat = (int (cdecl *)(int)) ptr2;
								}

							// set_midi_map
							FP_OFF(ptr1) += sizeof(int);
							FP_OFF(ptr2) = *(int *) ptr1;
							if (FP_OFF(ptr2))
								{
								set_midi_map = (int (cdecl *)(int)) ptr2;
								}

							// get_src_vol
							FP_OFF(ptr1) += sizeof(int);
							FP_OFF(ptr2) = *(int *) ptr1;
							if (FP_OFF(ptr2))
								{
								get_src_vol = (int (cdecl *)(int)) ptr2;
								}

							// set_src_vol
							FP_OFF(ptr1) += sizeof(int);
							FP_OFF(ptr2) = *(int *) ptr1;
							if (FP_OFF(ptr2))
								{
								set_src_vol = (int (cdecl *)(int, int)) ptr2;
								}

							// set_fade_pan
							FP_OFF(ptr1) += sizeof(int);
							FP_OFF(ptr2) = *(int *) ptr1;
							if (FP_OFF(ptr2))
								{
								set_fade_pan = (int (cdecl *)(void *)) ptr2;
								}

							// strt_fade_pan
							FP_OFF(ptr1) += sizeof(int);
							FP_OFF(ptr2) = *(int *) ptr1;
							if (FP_OFF(ptr2))
								{
								strt_fade_pan = ptr2;
								}

							// stop_fade_pan
							FP_OFF(ptr1) += sizeof(int);
							FP_OFF(ptr2) = *(int *) ptr1;
							if (FP_OFF(ptr2))
								{
								stop_fade_pan = (int (cdecl *)(int)) ptr2;
								}

							// pse_fade_pan
							FP_OFF(ptr1) += sizeof(int);
							FP_OFF(ptr2) = *(int *) ptr1;
							if (FP_OFF(ptr2))
								{
								pse_fade_pan = ptr2;
								}

							// res_fade_pan
							FP_OFF(ptr1) += sizeof(int);
							FP_OFF(ptr2) = *(int *) ptr1;
							if (FP_OFF(ptr2))
								{
								res_fade_pan = ptr2;
								}

							// read_fade_pan
							FP_OFF(ptr1) += sizeof(int);
							FP_OFF(ptr2) = *(int *) ptr1;
							if (FP_OFF(ptr2))
								{
								read_fade_pan = (int (cdecl *)(int)) ptr2;
								}

							// get_pan_pos
							FP_OFF(ptr1) += sizeof(int);
							FP_OFF(ptr2) = *(int *) ptr1;
							if (FP_OFF(ptr2))
								{
								get_pan_pos = (int (cdecl *)(int)) ptr2;
								}

							// set_pan_pos
							FP_OFF(ptr1) += sizeof(int);
							FP_OFF(ptr2) = *(int *) ptr1;
							if (FP_OFF(ptr2))
								{
								set_pan_pos = (int (cdecl *)(int, int)) ptr2;
								}

							// say_ascii
							FP_OFF(ptr1) += sizeof(int);
							FP_OFF(ptr2) = *(int *) ptr1;
							if (FP_OFF(ptr2))
								{
								say_ascii = (int (cdecl *)(const char *, int)) ptr2;
								}

							close(po);

							(*init_sound)();
							}
						else
							{
							close(po);
							printf(getcfgmsg(222), getcfgmsg(225));
							DeinitializeTimer();
							critical(FALSE);
							exit(200);
							}
						}
					}
				}
			else if (SameString(getcfgmsg(189), words[0]))
				{
				// do ups driver here
				if (upsDriver)
					{
					printf(getcfgmsg(189), getcfgmsg(224), bn);
					}
				else
					{
					strcpy(to, words[1]);
					strcat(to, getcfgmsg(194));
					if ((po = open(to, O_RDONLY | O_BINARY)) > 0)
						{
						long fl = filelength(po);

						upsDriver = new char[(uint) fl];

						if (upsDriver)
							{
							void *ptr1;
							int (cdecl *ptr2)(void);

							read(po, upsDesc, LABELSIZE);

							lseek(po, FP_OFF(upsDriver), SEEK_SET);
							read(po, upsDriver, (uint) (fl - FP_OFF(upsDriver)));

							ptr1 = upsDriver;
							ptr2 = (int (cdecl *)(void)) upsDriver;

							// init_ups
							FP_OFF(ptr1) = 32;
							FP_OFF(ptr2) = *(int *) ptr1;
							if (FP_OFF(ptr2))
								{
								initups = (void (cdecl *)(void)) ptr2;
								}

							// close_ups
							FP_OFF(ptr1) += sizeof(int);
							FP_OFF(ptr2) = *(int *) ptr1;
							if (FP_OFF(ptr2))
								{
								deinitups = (void (cdecl *)(void)) ptr2;
								}

							// statups
							FP_OFF(ptr1) += sizeof(int);
							FP_OFF(ptr2) = *(int *) ptr1;
							if (FP_OFF(ptr2))
								{
								statups = ptr2;
								}

							close(po);

							(*initups)();
							}
						else
							{
							close(po);
							printf(getcfgmsg(222), getcfgmsg(224));
							DeinitializeTimer();
							critical(FALSE);
							exit(200);
							}
						}
					}
				}
			else
				{
				printf(getmsg(21), citfiles[C_HARDWARE_CIT], words[0]);
				printf(bn);
				}
			}

		fclose(co);
		}
#endif
	}
