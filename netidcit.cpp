#include "ctdl.h"
#include "net.h"
#include "cfgfiles.h"
#include "miscovl.h"
#include "extmsg.h"


#ifdef WINCIT
#define doccr()
#endif

// checks NETID.CIT for existance of a net id
Bool IsNetIDInNetIDCit(const char *id)
	{
	assert(id);

	Bool found = FALSE;

	if (read_net_messages())
		{
		if (*id)
			{
			FILE *fBuf;
			char line[256];
			char *words[256];
			char path[80];

			sprintf(path, sbs, cfg.homepath, citfiles[C_NETID_CIT]);

			if ((fBuf = fopen(path, FO_R)) == NULL)
				{
				dump_net_messages();
				return (FALSE);
				}

			while (!found && (fgets(line, 255, fBuf) != NULL))
				{
				if (strnicmp(line, getnetmsg(88), strlen(getnetmsg(88))) != SAMESTRING)
					{
					continue;
					}

				if (parse_it(words, line) > 1 && SameString(getnetmsg(88), words[0]) && SameString(id, words[1]))
					{
					found = TRUE;
					}
				}

			fclose(fBuf);
			}

		dump_net_messages();
		}
	else
		{
		cOutOfMemory(75);
		}

	return (found);
	}


Bool AddNetIDToNetIDCit(const char *NetID, const char *Comment)
	{
	Bool Added = FALSE;

	if (read_net_messages())
		{
		char path[128];

		sprintf(path, sbs, cfg.homepath, citfiles[C_NETID_CIT]);

		Bool DidExist = filexists(path);

		FILE *file;

		if ((file = fopen(path, FO_A)) == NULL)
			{
#ifdef WINCIT
			errorDisp(getmsg(78), path);
#else
            CRmPrintf((getmsg(78), path));
#endif
			doccr();
			}
		else
			{
			if (!DidExist)
				{
				fprintf(file, getnetmsg(182));
				fprintf(file, bn);
				fprintf(file, getnetmsg(183));
				fprintf(file, bn);
				fprintf(file, bn);
				fprintf(file, getnetmsg(184));
				fprintf(file, bn);
				}

			char string[256];
			char string1[2*LABELSIZE+10];
			char string2[2*LABELSIZE+10];

			mkDblBck(NetID, string);
			sprintf(string1, getmsg(306), string);

			mkDblBck(Comment, string);
			sprintf(string2, getmsg(306), string);

			sprintf(string, getnetmsg(3), bn, getnetmsg(88), string1, string2);
			if (fwrite(string, sizeof(char), strlen(string), file) == strlen(string))
				{
				Added = TRUE;
				}

			fclose(file);
			}

		dump_net_messages();
		}
	else
		{
		cOutOfMemory(76);
		}

	return (Added);
	}

Bool RemoveNetIDFromNetIDCit(const char *NetID)
	{
	Bool Removed = FALSE, DiskFull = FALSE;

	if (read_net_messages())
		{
		char pathCit[80];
		FILE *Cit;

		sprintf(pathCit, sbs, cfg.homepath, citfiles[C_NETID_CIT]);

		if ((Cit = fopen(pathCit, FO_R)) != NULL)
			{
			char pathTmp[80];
			FILE *Tmp;

			sprintf(pathTmp, sbs, cfg.homepath, getnetmsg(89));

			if ((Tmp = fopen(pathTmp, FO_W)) != NULL)
				{
				char line[256];
				char *words[256];

				while ((fgets(line, 255, Cit) != NULL))
					{
					char line2[256];

					strcpy(line2, line);

					if (parse_it(words, line) < 2 ||
							!(SameString(getnetmsg(88), words[0]) && SameString(NetID, words[1])))
						{
						if (fwrite(line2, sizeof(char), strlen(line2), Tmp) != strlen(line2))
							{
#ifdef WINCIT
							errorDisp(getmsg(661), pathTmp);
#else
                            CRmPrintf((getmsg(661), pathTmp));
#endif
							doccr();
							DiskFull = TRUE;
							}
						}
					else
						{
						Removed = TRUE;
						}
					}

				fclose(Tmp);
				}
			else
				{
#ifdef WINCIT
				errorDisp(getmsg(8), pathTmp);
#else
                CRmPrintf(getmsg(8), pathTmp);
#endif
				doccr();
				}

			fclose(Cit);

			if (Removed && !DiskFull)
				{
				if (unlink(pathCit))	// 0 if good
					{
#ifdef WINCIT
					errorDisp(getmsg(662), pathCit);
#else
                    CRmPrintf(getmsg(662), pathCit);
#endif
					doccr();
					Removed = FALSE;
					}
				else
					{
					if (rename(pathTmp, pathCit))	// 0 if good
						{
#ifdef WINCIT
						errorDisp(getmsg(80), pathTmp);
#else
                        CRmPrintf(getmsg(80), pathTmp);
#endif
						doccr();
						}
					}
				}
			}
		else
			{
#ifdef WINCIT
			errorDisp(getmsg(78), pathCit);
#else
            CRmPrintf(getmsg(78), pathCit);
#endif
			doccr();
			}

		dump_net_messages();
		}
	else
		{
		cOutOfMemory(76);
		}

	return (Removed);
	}

Bool GetCommentOfNetIDInNetIDCit(const char *NetID, char *Comment, int Len)
	{
	Bool Found = FALSE;
	*Comment = 0;

	if (read_net_messages())
		{
		char pathCit[80];
		FILE *Cit;

		sprintf(pathCit, sbs, cfg.homepath, citfiles[C_NETID_CIT]);

		if ((Cit = fopen(pathCit, FO_R)) != NULL)
			{
			char line[256];
			char *words[256];

			while (!Found && (fgets(line, 255, Cit) != NULL))
				{
				if (strnicmp(line, getnetmsg(88), strlen(getnetmsg(88))) != SAMESTRING)
					{
					continue;
					}

				int count = parse_it(words, line);

				if (count > 1 && SameString(getnetmsg(88), words[0]) && SameString(NetID, words[1]))
					{
					Found = TRUE;

					if (count > 2)
						{
						CopyString2Buffer(Comment, words[2], Len);
						}
					}
				}

			fclose(Cit);
			}
		else
			{
#ifdef WINCIT
			errorDisp(getmsg(78), pathCit);
#else
            CRmPrintf(getmsg(78), pathCit);
#endif
			doccr();
			}

		dump_net_messages();
		}
	else
		{
		cOutOfMemory(76);
		}

	return (Found);
	}
