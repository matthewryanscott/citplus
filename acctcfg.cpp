// --------------------------------------------------------------------------
// Citadel: AcctCfg.CPP
//
// Time accounting code for Citadel bulletin board system.

#include "ctdl.h"
#pragma hdrstop

#include "account.h"
#include "log.h" 
#include "group.h"
#include "cfgfiles.h"
#include "extmsg.h"
#include "config.h"


// --------------------------------------------------------------------------
// ReadGrpdataCit(): Reads grpdata.cit values into group structure.
void ReadGrpdataCit(WC_TW)
	{
	FILE *fBuf;
	g_slot GroupSlot;

	if (!AccountingData &&
			(AccountingData = new AccountInfo[cfg.maxgroups]) == NULL)
		{
		crashout(getcfgmsg(66));
		}

	// initialize all accounting data
	for (GroupSlot = 0; GroupSlot < cfg.maxgroups; GroupSlot++)
		{
		// init days
		for (int i = 0; i < 7; i++)
			{
			AccountingData[GroupSlot].SetDay(i, TRUE);
			}

		// init hours & special hours
		for (int i1 = 0; i1 < 24; i1++)
			{
			AccountingData[GroupSlot].SetHour(i1, TRUE);
			AccountingData[GroupSlot].SetSpecialHour(i1, FALSE);
			}

		AccountingData[GroupSlot].SetDownloadMultiplier(-100);
		AccountingData[GroupSlot].SetUploadMultiplier(100);
		}

	char pathToGrpdataCit[128];
	sprintf(pathToGrpdataCit, sbs, cfg.homepath, citfiles[C_GRPDATA_CIT]);

	if ((fBuf = fopen(pathToGrpdataCit, FO_R)) == NULL) // ASCII mode
		{
		mPrintfCROrUpdateCfgDlg(TW, getmsg(15), pathToGrpdataCit);
		}
	else
		{
		discardable *d = readData(0, 8, 8);

		if (d)
			{
			char line[90];

			const char **grpkeywords = (const char **) d->aux;

			GroupSlot = CERROR;

			while (fgets(line, 90, fBuf) != NULL)
				{
				char *words[256];

				if (line[0] != '#')
					{
					continue;
					}

				int count = parse_it(words, line);

				int i;
				for (i = 0; i < GRK_NUM; i++)
					{
					if (SameString(words[0] + 1, grpkeywords[i]))
						{
						break;
						}
					}

				switch (i)
					{
					case GRK_DAYS:
						{
						int j;

						if (GroupSlot == CERROR)
							{
							break;
							}

						// init days
						for (j = 0; j < 7; j++)
							{
							AccountingData[GroupSlot].SetDay(j, FALSE);
							}

						for (j = 1; j < count; j++)
							{
							int k;

							for (k = 0; k < 7; k++)
								{
								if (SameString(words[j], days[k]) ||
										SameString(words[j], fulldays[k]))
									{
									break;
									}
								}

							if (k < 7)
								{
								AccountingData[GroupSlot].SetDay(k, TRUE);
								}
							else if (SameString(words[j], getcfgmsg(76))) // ANY
								{
								// Years of memory corruption
								// for (l = 0; l < MAXGROUPS; ++l)
								for (int l = 0; l < 7; ++l)
									{
									AccountingData[GroupSlot].SetDay(l, TRUE);
									}
								}
							else
								{
								CRmPrintfCROrUpdateCfgDlg(TW, getcfgmsg(77),
										citfiles[C_GRPDATA_CIT],
										getcfgmsg(78), words[j]);
								}
							}
						break;
						}

					case GRK_GROUP:
						{
						if ((GroupSlot = FindGroupByName(words[1])) != CERROR)
							{
							AccountingData[GroupSlot].SetAccountingOn(TRUE);
							}
						else
							{
							CRmPrintfCROrUpdateCfgDlg(TW, getcfgmsg(77),
									citfiles[C_GRPDATA_CIT],
									cfg.Lgroup_nym, words[1]);
							}

						break;
						}

					case GRK_HOURS:
						{
						int j;

						if (GroupSlot == CERROR)
							{
							break;
							}

						// init hours
						for (j = 0; j < 24; j++)
							{
							AccountingData[GroupSlot].SetHour(j, FALSE);
							}

						for (j = 1; j < count; j++)
							{
							if (SameString(words[j], getcfgmsg(76)))
								{
								int l;

								for (l = 0; l < 24; l++)
									{
									AccountingData[GroupSlot].SetHour(l, TRUE);
									}
								}
							else
								{
								int hour = atoi(words[j]);

								if (hour > 23)
									{
									CRmPrintfCROrUpdateCfgDlg(TW,
											getcfgmsg(82),
											citfiles[C_GRPDATA_CIT], ns,
											hour);
									}
								else
									{
									AccountingData[GroupSlot].SetHour(hour, TRUE);
									}
								}
							}
						break;
						}

					case GRK_DAYINC:
						{
						if (GroupSlot == CERROR)
							{
							break;
							}

						if (count > 1)
							{
							AccountingData[GroupSlot].SetDailyIncrement(60 * atol(words[1]));
							}
						break;
						}

					case GRK_DLMULT:
						{
						if (GroupSlot == CERROR)
							{
							break;
							}

						if (count > 1)
							{
							AccountingData[GroupSlot].SetDownloadMultiplier(atoi(words[1]));
							}
						break;
						}

					case GRK_ULMULT:
						{
						if (GroupSlot == CERROR)
							{
							break;
							}

						if (count > 1)
							{
							AccountingData[GroupSlot].SetUploadMultiplier(atoi(words[1]));
							}
						break;
						}

					case GRK_PRIORITY:
						{
						if (GroupSlot == CERROR)
							{
							break;
							}

						if (count > 1)
							{
							AccountingData[GroupSlot].SetPriority(atoi(words[1]));
							}

						break;
						}

					case GRK_MAXBAL:
						{
						if (GroupSlot == CERROR)
							{
							break;
							}

						if (count > 1)
							{
							AccountingData[GroupSlot].SetMaximumBalance(60 * atol(words[1]));
							}

						break;
						}

					case GRK_SPECIAL:
						{
						int j;

						if (GroupSlot == CERROR)
							{
							break;
							}

						// init hours
						for (j = 0; j < 24; j++)
							{
							AccountingData[GroupSlot].SetSpecialHour(j, FALSE);
							}

						for (j = 1; j < count; j++)
							{
							if (SameString(words[j], getcfgmsg(76)))
								{
								int l;

								for (l = 0; l < 24; l++)
									{
									AccountingData[GroupSlot].SetSpecialHour(l, TRUE);
									}
								}
							else
								{
								int hour;

								if ((hour = atoi(words[j])) > 23)
									{
									CRmPrintfCROrUpdateCfgDlg(TW,
											getcfgmsg(82),
											citfiles[C_GRPDATA_CIT],
											getcfgmsg(83), hour);
									}
								else
									{
									AccountingData[GroupSlot].SetSpecialHour(hour, TRUE);
									}
								}
							}
						break;
						}

					default:
						{
						CRmPrintfCROrUpdateCfgDlg(TW, getmsg(21),
								citfiles[C_GRPDATA_CIT], words[0]);
						break;
						}
					}
				}

			discardData(d);
			}
		else
			{
			OutOfMemory(56);
			}

		fclose(fBuf);
		}
	}
