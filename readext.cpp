// --------------------------------------------------------------------------
// Citadel: ReadExt.CPP
//
// Code to read EXTERNAL.CIT (and others)

#include "ctdl.h"
#pragma hdrstop

#include "extedit.h"
#include "cfgfiles.h"
#include "mdmreslt.h"
#include "events.h"
#include "extmsg.h"
#include "miscovl.h"
#include "config.h"


#ifdef WINCIT
static const char *ltoac(int wow)
    {
    static label Buffer;
    sprintf(Buffer, "%d", wow);
    return (Buffer);
    }
#endif


// --------------------------------------------------------------------------
// ReadExternalCit(): Reads EXTERNAL.CIT values into lists.

void ReadExternalCit(WC_TW)
    {
    strftime(NULL, 0, NULL, -1);

    ForgetAllExternalEditors();

    disposeLL((void **) &holidayList);
    disposeLL((void **) &userAppList);
    disposeLL((void **) &extDoorList);
    disposeLL((void **) &censorList);
    disposeLL((void **) &replaceList);
    disposeLL((void **) &eventList);
    disposeLL((void **) &arcList);
    disposeLL((void **) &netCmdList);

    char fullPathToExternalCit[128];
    sprintf(fullPathToExternalCit, sbs, cfg.homepath, citfiles[C_EXTERNAL_CIT]);

    FILE *fBuf;
    if ((fBuf = fopen(fullPathToExternalCit, FO_R)) == NULL)    // ASCII mode
        {
        mPrintfCROrUpdateCfgDlg(TW, getmsg(15), fullPathToExternalCit);
        }
    else
        {
        compactMemory(1);

        discardable *d = readData(0, EXTERNALDDSTART, EXTERNALDDEND);

        if (d)
            {
            int lineNo = 0;
            const char **extkeywords = (const char **) d->aux;

            char line[257];
            while (fgets(line, sizeof(line) - 1, fBuf) != NULL)
                {
                lineNo++;

                if (line[0] != '#')
                    {
                    continue;
                    }

                char *words[256];
                const int count = parse_it(words, line);

                int i;
                for (i = 0; i < EC_NUM; i++)
                    {
                    if (SameString(words[0], extkeywords[i]))
                        {
                        break;
                        }
                    }

                switch (i)
                    {
                    case EC_EDITOR:
                        {
                        const Bool cnfrm = (count > 4) ? !!atoi(words[4]) : FALSE;

                        if (!AddExternalEditor(words[1], words[3], !!atoi(words[2]), cnfrm, FALSE))
                            {
                            CRmPrintfOrUpdateCfgDlg(TW, getcfgmsg(75), extkeywords[i], words[1]);
                            }

                        break;
                        }

                    case EC_USERAPP:
                        {
                        if (strlen(words[1]) > sizeof(userAppList->name) - 1)
                            {
                            mPrintfOrUpdateCfgDlg(TW, getmsg(102), getcfgmsg(79), sizeof(userAppList->name));
                            break;
                            }

                        if (strlen(words[2]) > sizeof(userAppList->outstr) - 1)
                            {
                            mPrintfOrUpdateCfgDlg(TW, getmsg(102), getcfgmsg(80), sizeof(userAppList->outstr));
                            break;
                            }

                        if (strlen(words[3]) > sizeof(userAppList->cmd) - 1)
                            {
                            mPrintfOrUpdateCfgDlg(TW, getmsg(102), getcfgmsg(81), sizeof(userAppList->cmd));
                            break;
                            }

                        userapps *theUserapp = (userapps *) addLL((void **) &userAppList, sizeof(*theUserapp));

                        if (theUserapp)
                            {
                            strcpy(theUserapp->name, words[1]);
                            strcpy(theUserapp->outstr, words[2]);
                            strcpy(theUserapp->cmd, words[3]);
                            theUserapp->hangup = atoi(words[4]);
                            }
                        else
                            {
                            CRmPrintfOrUpdateCfgDlg(TW, getcfgmsg(75), extkeywords[i], words[1]);
                            }

                        break;
                        }

                    case EC_AUTO_EDITOR:
                        {
                        const Bool cnfrm = (count > 4) ? !!atoi(words[4]) : FALSE;

                        if (!AddExternalEditor(words[1], words[3], !!atoi(words[2]), cnfrm, TRUE))
                            {
                            CRmPrintfOrUpdateCfgDlg(TW, getcfgmsg(75), extkeywords[i], words[1]);
                            }

                        break;
                        }

                    case EC_REPLACE:
                        {
                        if (strlen(words[1]) > sizeof(replaceList->string1) - 1)
                            {
                            mPrintfOrUpdateCfgDlg(TW, getmsg(102), getcfgmsg(84), sizeof(replaceList->string1));
                            break;
                            }

                        if (strlen(words[2]) > sizeof(replaceList->string2) - 1)
                            {
                            mPrintfOrUpdateCfgDlg(TW, getmsg(102), getcfgmsg(85), sizeof(replaceList->string2));
                            break;
                            }

                        pairedStrings *thisReplace = (pairedStrings *) addLL((void **) &replaceList, sizeof(*thisReplace));

                        if (thisReplace)
                            {
                            strcpy(thisReplace->string1, words[1]);
                            strcpy(thisReplace->string2, words[2]);
                            }
                        else
                            {
                            CRmPrintfOrUpdateCfgDlg(TW, getcfgmsg(75), extkeywords[i], words[1]);
                            }

                        break;
                        }

                    case EC_DOOR:
                        {
                        if (count < 8)
                            {
                            CRmPrintfOrUpdateCfgDlg(TW, getcfgmsg(86), extkeywords[i]);
                            break;
                            }

                        if (strlen(words[1]) > sizeof(extDoorList->name) - 1)
                            {
                            mPrintfOrUpdateCfgDlg(TW, getmsg(102), getcfgmsg(87), sizeof(extDoorList->name));
                            break;
                            }

                        if (strlen(words[2]) > sizeof(extDoorList->cmd) - 1)
                            {
                            mPrintfOrUpdateCfgDlg(TW, getmsg(102), getcfgmsg(88), sizeof(extDoorList->cmd));
                            break;
                            }

                        if (strlen(words[3]) > sizeof(extDoorList->group) - 1)
                            {
                            char Wow[128];

                            sprintf(Wow, getcfgmsg(89), cfg.Lgroup_nym);
                            mPrintfOrUpdateCfgDlg(TW, getmsg(102), Wow, sizeof(extDoorList->group));
                            break;
                            }

                        doors *theDoor = (doors *) addLL((void **) &extDoorList, sizeof(*theDoor));

                        if (theDoor)
                            {
                            strcpy(theDoor->name, words[1]);
                            strcpy(theDoor->cmd, words[2]);
                            strcpy(theDoor->group, words[3]);
                            theDoor->CON = atoi(words[4]);
                            theDoor->SYSOP = atoi(words[5]);
                            theDoor->AIDE = atoi(words[6]);
                            theDoor->DIR = atoi(words[7]);
                            if (count > 8)
                                {
                                theDoor->WHERE = atoi(words[8]);
                                }
                            else
                                {
                                theDoor->WHERE = 3; //default
                                }
                            }
                        else
                            {
                            CRmPrintfOrUpdateCfgDlg(TW, getcfgmsg(75), extkeywords[i], words[1]);
                            }

                        break;
                        }

                    case EC_C_AUTHOR:
                    case EC_C_NODE:
                    case EC_C_TEXT:
                        {
                        if (strlen(words[1]) > sizeof(censorList->str) - 1)
                            {
                            mPrintfOrUpdateCfgDlg(TW, getmsg(102), extkeywords[i], sizeof(censorList->str));
                            }
                        else
                            {
                            censor *thisCensor = (censor *) addLL((void **) &censorList, sizeof(*censorList));

                            if (!thisCensor)
                                {
                                CRmPrintfOrUpdateCfgDlg(TW, getcfgmsg(75), extkeywords[i], words[1]);
                                }
                            else
                                {
                                if (i == EC_C_TEXT)
                                    {
                                    thisCensor->what = CENSOR_TEXT;
                                    }
                                else if (i == EC_C_AUTHOR)
                                    {
                                    thisCensor->what = CENSOR_AUTHOR;
                                    }
                                else if (i == EC_C_NODE)
                                    {
                                    thisCensor->what = CENSOR_NODE;
                                    }

                                strcpy(thisCensor->str, words[1]);
                                }
                            }

                        break;
                        }

                    case EC_HOLIDAY:
                    case EC_HOLIDAYWITHYEAR:
                        {
                        if (strlen(words[i == EC_HOLIDAY ? 5 : 6]) > sizeof(holidayList->name) - 1)
                            {
                            mPrintfOrUpdateCfgDlg(TW, getmsg(102), getcfgmsg(91), sizeof(holidayList->name));
                            break;
                            }

                        if (atoi(words[1]) < 1 || atoi(words[1]) > 12)
                            {
                            mPrintfOrUpdateCfgDlg(TW, getcfgmsg(92));
                            break;
                            }

                        if (atoi(words[2]) < 0 || atoi(words[2]) > 31)
                            {
                            mPrintfOrUpdateCfgDlg(TW, getcfgmsg(93));
                            break;
                            }

                        if (atoi(words[3]) < 0 || atoi(words[3]) > 5)
                            {
                            mPrintfOrUpdateCfgDlg(TW, getcfgmsg(94));
                            break;
                            }

                        if (atoi(words[4]) < 0 || atoi(words[4]) > 7)
                            {
                            mPrintfOrUpdateCfgDlg(TW, getcfgmsg(95));
                            break;
                            }

                        if ((atoi(words[4]) || atoi(words[3])) && !(atoi(words[4]) && atoi(words[3])))
                            {
                            mPrintfOrUpdateCfgDlg(TW, getcfgmsg(96));
                            break;
                            }

                        if (atoi(words[2]) && (atoi(words[4]) || atoi(words[3])))
                            {
                            mPrintfOrUpdateCfgDlg(TW, getcfgmsg(97));
                            break;
                            }

                        holidays *thisHol = (holidays *) addLL((void **) &holidayList, sizeof(*holidayList));

                        if (thisHol)
                            {
                            if (i == EC_HOLIDAY)
                                {
                                thisHol->year = -1;
                                strcpy(thisHol->name, words[5]);
                                }
                            else
                                {
                                thisHol->year = atoi(words[5]);
                                if (thisHol->year < 1900)
                                    {
                                    thisHol->year += 1900;
                                    }
                                if (thisHol->year < 1939)
                                    {
                                    thisHol->year += 100;
                                    }

                                strcpy(thisHol->name, words[6]);
                                }

                            thisHol->Month = EvaluateMonth(words[1]);
                            thisHol->date = atoi(words[2]);
                            thisHol->week = atoi(words[3]);
                            thisHol->day = EvaluateDay(words[4]);
                            }
                        else
                            {
                            CRmPrintfOrUpdateCfgDlg(TW, getcfgmsg(75), extkeywords[i], words[5]);
                            }

                        break;
                        }

                    case EC_EVENT:
                        {
                        int j;
                        const char **eventkeywords = (const char **) d->next->aux;

                        for (j = 0; j < EVT_NUMEVTS; j++)
                            {
                            if (SameString(words[1], eventkeywords[j]))
                                {
                                break;
                                }
                            }

                        if (j < EVT_NUMEVTS)
                            {
                            events *theEvent = (events *) addLL((void **) &eventList, sizeof(*theEvent) + strlen(words[2]));

                            if (theEvent)
                                {
                                theEvent->type = (CITEVENTS) j;
                                strcpy(theEvent->line, words[2]);
                                }
                            else
                                {
                                CRmPrintfOrUpdateCfgDlg(TW, getcfgmsg(75), words[0], words[2]);
                                }
                            }
                        else
                            {
                            CRmPrintfOrUpdateCfgDlg(TW, getcfgmsg(77), citfiles[C_EXTERNAL_CIT], extkeywords[i], words[1]);
                            }

                        break;
                        }

                    case EC_ARCHIVER:
                        {
                        if (count > 4)
                            {
                            if (strlen(words[1]) > sizeof(arcList->name) - 1)
                                {
                                mPrintfOrUpdateCfgDlg(TW, getmsg(102), getcfgmsg(102), sizeof(arcList->name));
                                break;
                                }

                            if (strlen(words[2]) > sizeof(arcList->view) - 1)
                                {
                                mPrintfOrUpdateCfgDlg(TW, getmsg(102), getcfgmsg(103), sizeof(arcList->view));
                                break;
                                }

                            if (strlen(words[3]) > sizeof(arcList->extract) - 1)
                                {
                                mPrintfOrUpdateCfgDlg(TW, getmsg(102), getcfgmsg(104), sizeof(arcList->extract));
                                break;
                                }

                            if (strlen(words[4]) > sizeof(arcList->ext) - 1)
                                {
                                mPrintfOrUpdateCfgDlg(TW, getmsg(102), getcfgmsg(105), sizeof(arcList->ext));
                                break;
                                }

                            archivers *theArc = (archivers *) addLL((void **) &arcList, sizeof(*theArc));

                            if (theArc)
                                {
                                strcpy(theArc->name, words[1]);
                                strcpy(theArc->view, words[2]);
                                strcpy(theArc->extract, words[3]);
                                strcpy(theArc->ext, words[4]);
                                }
                            else
                                {
                                CRmPrintfOrUpdateCfgDlg(TW, getcfgmsg(75), words[0], words[1]);
                                }
                            }
                        else
                            {
                            CRmPrintfOrUpdateCfgDlg(TW, getcfgmsg(86), extkeywords[i]);
                            }

                        break;
                        }

                    case EC_DIRECTORY:
                    case EC_REFUSER:
                    case EC_BANPORT:
                        {
                        // read as needed...
                        break;
                        }

#ifndef WINCIT
                    case EC_RLM:
                        {
                        if (count > 1)
                            {
                            if (!initRLM(words[1]))
                                {
                                CRmPrintfOrUpdateCfgDlg(TW, getcfgmsg(108), words[1]);
                                }
                            }
                        else
                            {
                            CRmPrintfOrUpdateCfgDlg(TW, getcfgmsg(86), extkeywords[i]);
                            }

                        break;
                        }
#endif

                    case EC_NETCOMMAND:
                        {
                        if (count > 2)
                            {
                            pairedStrings *theCmd = (pairedStrings *) addLL((void **) &netCmdList, sizeof(pairedStrings));

                            if (theCmd)
                                {
                                CopyStringToBuffer(theCmd->string1, words[1]);
                                CopyStringToBuffer(theCmd->string2, words[2]);
                                }
                            else
                                {
                                CRmPrintfOrUpdateCfgDlg(TW, getcfgmsg(75), extkeywords[i], words[1]);
                                }
                            }
                        else
                            {
                            CRmPrintfOrUpdateCfgDlg(TW, getcfgmsg(86), extkeywords[i]);
                            }

                        break;
                        }

                    default:
                        {
                        mPrintfCROrUpdateCfgDlg(TW, getcfgmsg(109), citfiles[C_EXTERNAL_CIT], ltoac(lineNo), words[0]);
                        break;
                        }
                    }
                }

            discardData(d);
            }
        else
            {
            OutOfMemory(59);
            }

        fclose(fBuf);
        }
    }


// --------------------------------------------------------------------------
// ReadProtocolCit(): Reads PROTOCOL.CIT values into list.

void ReadProtocolCit(WC_TW)
    {
    protocols *theProt = NULL;

    disposeLL((void **) &extProtList);

    char fullPathToProtocolCit[128];
    sprintf(fullPathToProtocolCit, sbs, cfg.homepath, citfiles[C_PROTOCOL_CIT]);

    FILE *fBuf;
    if ((fBuf = fopen(fullPathToProtocolCit, FO_R)) == NULL)    // ASCII mode
        {
        mPrintfCROrUpdateCfgDlg(TW, getmsg(15), fullPathToProtocolCit);
        }
    else
        {
        compactMemory(1);

        discardable *d = readData(0, PROTOCOLDDSTART, PROTOCOLDDEND);

        if (d)
            {
            int lineNo = 0;
            const char **protkeywords = (const char **) d->aux;

            char line[257];
            while (fgets(line, sizeof(line) - 1, fBuf) != NULL)
                {
                lineNo++;

                if (line[0] != '#')
                    {
                    continue;
                    }

                char *words[256];
                const int count = parse_it(words, line);

                int i;
                for (i = 0; i < PR_NUM; i++)
                    {
                    if (SameString(words[0], protkeywords[i]))
                        {
                        break;
                        }
                    }

                switch (i)
                    {
                    case PR_PROTOCOL:
                        {
                        if (theProt)
                            {
                            if (!*theProt->MenuName)
                                {
                                CopyStringToBuffer(theProt->MenuName, theProt->name);
                                }

                            if (!theProt->CommandKey)
                                {
                                theProt->CommandKey = theProt->name[0];
                                }

                            if (!*theProt->autoDown)
                                {
                                theProt->autoDown[0] = 7;
                                }
                            }

                        theProt = NULL;

                        if (count > 1)
                            {
                            if (strlen(words[1]) > sizeof(theProt->name) - 1)
                                {
                                mPrintfCROrUpdateCfgDlg(TW, getcfgmsg(68), citfiles[C_PROTOCOL_CIT], ltoac(lineNo),
                                        protkeywords[i], sizeof(theProt->name));
                                }

                            theProt = (protocols *) addLL((void **) &extProtList, sizeof(*theProt));

                            if (theProt)
                                {
                                CopyStringToBuffer(theProt->name, words[1]);
                                }
                            else
                                {
                                mPrintfCROrUpdateCfgDlg(TW, getcfgmsg(75), protkeywords[i], words[1]);
                                }
                            }
                        else
                            {
                            mPrintfCROrUpdateCfgDlg(TW, getcfgmsg(164), citfiles[C_PROTOCOL_CIT], ltoac(lineNo),
                                    protkeywords[i]);
                            }

                        break;
                        }

                    case PR_MENU_NAME:
                        {
                        if (count > 1)
                            {
                            if (strlen(words[1]) > sizeof(theProt->MenuName) - 1)
                                {
                                mPrintfCROrUpdateCfgDlg(TW, getcfgmsg(68), citfiles[C_PROTOCOL_CIT], ltoac(lineNo),
                                        protkeywords[i], sizeof(theProt->MenuName));
                                }

                            if (theProt)
                                {
                                CopyStringToBuffer(theProt->MenuName, words[1]);
                                }
                            else
                                {
                                mPrintfCROrUpdateCfgDlg(TW, getcfgmsg(67), citfiles[C_PROTOCOL_CIT], ltoac(lineNo),
                                        protkeywords[i], protkeywords[PR_PROTOCOL]);
                                }
                            }
                        else
                            {
                            mPrintfCROrUpdateCfgDlg(TW, getcfgmsg(164), citfiles[C_PROTOCOL_CIT], ltoac(lineNo),
                                    protkeywords[i]);
                            }

                        break;
                        }

                    case PR_COMMAND_KEY:
                        {
                        if (count > 1)
                            {
                            if (theProt)
                                {
                                theProt->CommandKey = words[1][0];
                                }
                            else
                                {
                                mPrintfCROrUpdateCfgDlg(TW, getcfgmsg(67), citfiles[C_PROTOCOL_CIT], ltoac(lineNo),
                                        protkeywords[i], protkeywords[PR_PROTOCOL]);
                                }
                            }
                        else
                            {
                            mPrintfCROrUpdateCfgDlg(TW, getcfgmsg(164), citfiles[C_PROTOCOL_CIT], ltoac(lineNo),
                                    protkeywords[i]);
                            }

                        break;
                        }

                    case PR_BATCH:
                        {
                        if (count > 1)
                            {
                            if (theProt)
                                {
                                Bool Error;
                                Bool Value = GetBooleanFromCfgFile(words[1], &Error);

                                if (Error)
                                    {
                                    mPrintfCROrUpdateCfgDlg(TW, getcfgmsg(69), citfiles[C_PROTOCOL_CIT], ltoac(lineNo),
                                            protkeywords[i], words[1]);
                                    }
                                else
                                    {
                                    theProt->batch = Value;
                                    }
                                }
                            else
                                {
                                mPrintfCROrUpdateCfgDlg(TW, getcfgmsg(67), citfiles[C_PROTOCOL_CIT], ltoac(lineNo),
                                        protkeywords[i], protkeywords[PR_PROTOCOL]);
                                }
                            }
                        else
                            {
                            mPrintfCROrUpdateCfgDlg(TW, getcfgmsg(164), citfiles[C_PROTOCOL_CIT], ltoac(lineNo),
                                    protkeywords[i]);
                            }

                        break;
                        }

                    case PR_BLOCK_SIZE:
                        {
                        if (count > 1)
                            {
                            if (theProt)
                                {
                                if (isNumeric(words[1]))
                                    {
                                    theProt->block = atoi(words[1]);
                                    }
                                else
                                    {
                                    mPrintfCROrUpdateCfgDlg(TW, getcfgmsg(242), citfiles[C_PROTOCOL_CIT], ltoac(lineNo),
                                            protkeywords[i]);
                                    }
                                }
                            else
                                {
                                mPrintfCROrUpdateCfgDlg(TW, getcfgmsg(67), citfiles[C_PROTOCOL_CIT], ltoac(lineNo),
                                        protkeywords[i], protkeywords[PR_PROTOCOL]);
                                }
                            }
                        else
                            {
                            mPrintfCROrUpdateCfgDlg(TW, getcfgmsg(164), citfiles[C_PROTOCOL_CIT], ltoac(lineNo),
                                    protkeywords[i]);
                            }

                        break;
                        }

                    case PR_RECEIVE:
                        {
                        if (count > 1)
                            {
                            if (strlen(words[1]) > sizeof(theProt->rcv) - 1)
                                {
                                mPrintfCROrUpdateCfgDlg(TW, getcfgmsg(68), citfiles[C_PROTOCOL_CIT], ltoac(lineNo),
                                        protkeywords[i], sizeof(theProt->rcv));
                                }

                            if (theProt)
                                {
                                CopyStringToBuffer(theProt->rcv, words[1]);
                                }
                            else
                                {
                                mPrintfCROrUpdateCfgDlg(TW, getcfgmsg(67), citfiles[C_PROTOCOL_CIT], ltoac(lineNo),
                                        protkeywords[i], protkeywords[PR_PROTOCOL]);
                                }
                            }
                        else
                            {
                            mPrintfCROrUpdateCfgDlg(TW, getcfgmsg(164), citfiles[C_PROTOCOL_CIT], ltoac(lineNo),
                                    protkeywords[i]);
                            }

                        break;
                        }

                    case PR_SEND:
                        {
                        if (count > 1)
                            {
                            if (strlen(words[1]) > sizeof(theProt->snd) - 1)
                                {
                                mPrintfCROrUpdateCfgDlg(TW, getcfgmsg(68), citfiles[C_PROTOCOL_CIT], ltoac(lineNo),
                                        protkeywords[i], sizeof(theProt->snd));
                                }

                            if (theProt)
                                {
                                CopyStringToBuffer(theProt->snd, words[1]);
                                }
                            else
                                {
                                mPrintfCROrUpdateCfgDlg(TW, getcfgmsg(67), citfiles[C_PROTOCOL_CIT], ltoac(lineNo),
                                        protkeywords[i], protkeywords[PR_PROTOCOL]);
                                }
                            }
                        else
                            {
                            mPrintfCROrUpdateCfgDlg(TW, getcfgmsg(164), citfiles[C_PROTOCOL_CIT], ltoac(lineNo),
                                    protkeywords[i]);
                            }

                        break;
                        }

                    case PR_AUTO_UPLOAD:
                        {
                        if (count > 1)
                            {
                            if (strlen(words[1]) > sizeof(theProt->autoDown) - 1)
                                {
                                mPrintfCROrUpdateCfgDlg(TW, getcfgmsg(68), citfiles[C_PROTOCOL_CIT], ltoac(lineNo),
                                        protkeywords[i], sizeof(theProt->autoDown));
                                }

                            if (theProt)
                                {
                                CopyStringToBuffer(theProt->autoDown, words[1]);
                                }
                            else
                                {
                                mPrintfCROrUpdateCfgDlg(TW, getcfgmsg(67), citfiles[C_PROTOCOL_CIT], ltoac(lineNo),
                                        protkeywords[i], protkeywords[PR_PROTOCOL]);
                                }
                            }
                        else
                            {
                            mPrintfCROrUpdateCfgDlg(TW, getcfgmsg(164), citfiles[C_PROTOCOL_CIT], ltoac(lineNo),
                                    protkeywords[i]);
                            }

                        break;
                        }

                    case PR_RESPONSE_SEND:
                        {
                        if (count > 1)
                            {
                            if (strlen(words[1]) > sizeof(theProt->respDown) - 1)
                                {
                                mPrintfCROrUpdateCfgDlg(TW, getcfgmsg(68), citfiles[C_PROTOCOL_CIT], ltoac(lineNo),
                                        protkeywords[i], sizeof(theProt->respDown));
                                }

                            if (theProt)
                                {
                                CopyStringToBuffer(theProt->respDown, words[1]);
                                }
                            else
                                {
                                mPrintfCROrUpdateCfgDlg(TW, getcfgmsg(67), citfiles[C_PROTOCOL_CIT], ltoac(lineNo),
                                        protkeywords[i], protkeywords[PR_PROTOCOL]);
                                }
                            }
                        else
                            {
                            mPrintfCROrUpdateCfgDlg(TW, getcfgmsg(164), citfiles[C_PROTOCOL_CIT], ltoac(lineNo),
                                    protkeywords[i]);
                            }

                        break;
                        }

                    case PR_NET_ONLY:
                        {
                        if (count > 1)
                            {
                            if (theProt)
                                {
                                if (SameString(getmsg(627), words[1]) || SameString(getmsg(522), words[1]) ||
                                        SameString(getmsg(316), words[1]) || SameString(getmsg(688), words[1]) ||
                                        SameString(getmsg(690), words[1]))
                                    {
                                    theProt->NetOnly = FALSE;
                                    }
                                else if (SameString(getmsg(626), words[1]) || SameString(getmsg(521), words[1]) ||
                                        SameString(getmsg(317), words[1]) || SameString(getmsg(687), words[1]) ||
                                        SameString(getmsg(689), words[1]))
                                    {
                                    theProt->NetOnly = TRUE;
                                    }
                                else if (isNumeric(words[1]))
                                    {
                                    theProt->NetOnly = atoi(words[1]);
                                    theProt->NetOnly = !!theProt->NetOnly;
                                    }
                                else
                                    {
                                    mPrintfCROrUpdateCfgDlg(TW, getcfgmsg(69), citfiles[C_PROTOCOL_CIT], ltoac(lineNo),
                                            protkeywords[i], words[1]);
                                    }
                                }
                            else
                                {
                                mPrintfCROrUpdateCfgDlg(TW, getcfgmsg(67), citfiles[C_PROTOCOL_CIT], ltoac(lineNo),
                                        protkeywords[i], protkeywords[PR_PROTOCOL]);
                                }
                            }
                        else
                            {
                            mPrintfCROrUpdateCfgDlg(TW, getcfgmsg(164), citfiles[C_PROTOCOL_CIT], ltoac(lineNo),
                                        protkeywords[i]);
                            }

                        break;
                        }

                    case PR_CHECK_TRANSFER:
                        {
                        if (count > 2)
                            {
                            if (theProt)
                                {
                                theProt->CheckType = atoi(words[1]);

                                if (theProt->CheckType)
                                    {
                                    if (strlen(words[2]) > sizeof(theProt->CheckMethod) - 1)
                                        {
                                        mPrintfCROrUpdateCfgDlg(TW, getcfgmsg(68), citfiles[C_PROTOCOL_CIT], ltoac(lineNo),
                                                protkeywords[i], sizeof(theProt->CheckMethod));
                                        }

                                    CopyStringToBuffer(theProt->CheckMethod, words[2]);
                                    }
                                }
                            else
                                {
                                mPrintfCROrUpdateCfgDlg(TW, getcfgmsg(67), citfiles[C_PROTOCOL_CIT], ltoac(lineNo),
                                        protkeywords[i], protkeywords[PR_PROTOCOL]);
                                }
                            }
                        else
                            {
                            mPrintfCROrUpdateCfgDlg(TW, getcfgmsg(164), citfiles[C_PROTOCOL_CIT], ltoac(lineNo),
                                    protkeywords[i]);
                            }

                        break;
                        }

                    default:
                        {
                        mPrintfCROrUpdateCfgDlg(TW, getcfgmsg(109), citfiles[C_PROTOCOL_CIT], ltoac(lineNo), words[0]);
                        break;
                        }
                    }
                }

            if (theProt)
                {
                if (!*theProt->MenuName)
                    {
                    CopyStringToBuffer(theProt->MenuName, theProt->name);
                    }

                if (!theProt->CommandKey)
                    {
                    theProt->CommandKey = theProt->name[0];
                    }

                if (!*theProt->autoDown)
                    {
                    theProt->autoDown[0] = 7;
                    }
                }

            discardData(d);
            }
        else
            {
            OutOfMemory(60);
            }

        fclose(fBuf);
        }
    }


// --------------------------------------------------------------------------
// ReadMdmresltCit(): Reads MDMRESLT.CIT values into list.

#ifdef WINCIT
static void ParseMdmresltLine(TermWindowC *TW, char *InputLine, const char **Keywords, int LineNum)
#else
static void ParseMdmresltLine(int, char *InputLine, const char **Keywords, int LineNum)
#endif
    {
    char *words[256];

    const int count = parse_it(words, InputLine);

    int i;
    for (i = 0; i < MR_NUM; i++)
        {
        if (SameString(words[0], Keywords[i]))
            {
            break;
            }
        }

    if (i != MR_NUM)
        {
        if (count > 1)
            {
            ModemResultCodes *theCode = (ModemResultCodes *) addLL((void **) &ResultCodes, sizeof(ModemResultCodes) +
                    strlen(words[1]));

            if (theCode)
                {
                theCode->Which = (ResultCodeEnum) i;
                strcpy(theCode->Code, words[1]);
                if (MR_RUNAPPLIC == theCode->Which)
                    {
                    if (count > 2)
                        {
                        ModemResultCodes *theCode2 = (ModemResultCodes *) addLL((void **) &ResultCodes,
                                sizeof(ModemResultCodes) + strlen(words[2]));

                        if (theCode2)
                            {
                            theCode2->Which = MR_RUNAPPNAME;
                            strcpy(theCode2->Code, words[2]);
                            }
                        else
                            {
                            OutOfMemory(61);
                            }
                        }
                    else
                        {
                        mPrintfCROrUpdateCfgDlg(TW, getcfgmsg(164), citfiles[C_MDMRESLT_CIT], ltoac(LineNum), Keywords[i]);
                        }
                    }
                }
            else
                {
                OutOfMemory(61);
                }
            }
        else
            {
            mPrintfCROrUpdateCfgDlg(TW, getcfgmsg(164), citfiles[C_MDMRESLT_CIT], ltoac(LineNum), Keywords[i]);
            }
        }
    else
        {
        mPrintfCROrUpdateCfgDlg(TW, getcfgmsg(109), citfiles[C_MDMRESLT_CIT], ltoac(LineNum), words[0]);
        }
    }

void ReadMdmresltCit(WC_TW)
    {
    disposeLL((void **) &ResultCodes);

    compactMemory(1);

    discardable *d = readData(0, MDMRESLTDDSTART, MDMRESLTDDEND);

    if (d)
        {
        const char **mdmresltkeywords = (const char **) d->aux;

        char fullPathToMdmresltCit[128];
        sprintf(fullPathToMdmresltCit, sbs, cfg.homepath, citfiles[C_MDMRESLT_CIT]);

        FILE *fBuf;
        if ((fBuf = fopen(fullPathToMdmresltCit, FO_R)) == NULL) // ASCII mode
            {
            mPrintfCROrUpdateCfgDlg(TW, getcfgmsg(128), fullPathToMdmresltCit);

            // Make default...
            discardable *d2 = readData(16, 4, 4);

            if (d2)
                {
                for (int i = 0; !SameString(((const char **) d2->aux)[i], getmsg(307)); i++)
                    {
                    char line[257];

                    if (((const char **) d2->aux)[i][0] == '#')
                        {
                        strcpy(line, ((const char **) d2->aux)[i]);
                        ParseMdmresltLine(TW, line, mdmresltkeywords, -1);
                        }
                    }

                discardData(d2);
                }
            else
                {
                OutOfMemory(62);
                }
            }
        else
            {
            char line[257];
            int lineNo = 0;

            while (fgets(line, sizeof(line) - 1, fBuf) != NULL)
                {
                lineNo++;

                if (line[0] != '#')
                    {
                    continue;
                    }

                ParseMdmresltLine(TW, line, mdmresltkeywords, lineNo);
                }

            fclose(fBuf);
            }

        discardData(d);
        }
    else
        {
        OutOfMemory(62);
        }
    }


// --------------------------------------------------------------------------
// ReadCommandsCit(): Reads COMMANDS.CIT values into list.

void ReadCommandsCit(WC_TW)
    {
    memset(CommandOverrides, 0, sizeof(CommandOverrides));

    char fullPathToCommandsCit[128];
    sprintf(fullPathToCommandsCit, sbs, cfg.homepath, citfiles[C_COMMANDS_CIT]);

    FILE *fBuf;
    if ((fBuf = fopen(fullPathToCommandsCit, FO_R)) == NULL)    // ASCII mode
        {
        mPrintfCROrUpdateCfgDlg(TW, getmsg(15), fullPathToCommandsCit);
        }
    else
        {
        discardable *d = readData(0, COMMANDSDDSTART, COMMANDSDDEND);

        if (d)
            {
            int lineNo = 0;
            const char **cmdkeywords = (const char **) d->aux;

            char line[257];
            while (fgets(line, sizeof(line) - 1, fBuf) != NULL)
                {
                lineNo++;

                if (line[0] != '#')
                    {
                    continue;
                    }

                char *words[256];
                const int count = parse_it(words, line);

                int i;
                for (i = 0; i < UC_NUM; i++)
                    {
                    if (SameString(words[0] + 1, cmdkeywords[i]))
                        {
                        break;
                        }
                    }

                if (i != UC_NUM)
                    {
                    if (count == 2)
                        {
                        CopyStringToBuffer(CommandOverrides[i], words[1]);
                        }
                    else
                        {
                        mPrintfCROrUpdateCfgDlg(TW, getcfgmsg(213), citfiles[C_COMMANDS_CIT], ltoac(lineNo), words[0]);
                        }
                    }
                else
                    {
                    mPrintfCROrUpdateCfgDlg(TW, getcfgmsg(109), citfiles[C_COMMANDS_CIT], ltoac(lineNo), words[0]);
                    }
                }

            discardData(d);
            }
        else
            {
            OutOfMemory(116);
            }

        fclose(fBuf);
        }
    }


// --------------------------------------------------------------------------
// ReadMCICit(): Reads MCI.CIT values.
#ifdef WINCIT
void ReadMCICit(Bool TellIfNotFound, WC_TW)
#else
void ReadMCICit(Bool TellIfNotFound)
#endif
    {
    tw()MCIRecursionChecker.EnableAll();

    char fullPathToMCICit[128];
    sprintf(fullPathToMCICit, sbs, cfg.homepath, citfiles[C_MCI_CIT]);

    FILE *fBuf;
    if ((fBuf = fopen(fullPathToMCICit, FO_R)) != NULL) // ASCII mode
        {
        discardable *d = readData(0, MCIDDSTART, MCIDDEND);

        if (d)
            {
            int lineNo = 0;
            const char **mcikeywords = (const char **) d->aux;

            char line[257];
            while (fgets(line, sizeof(line) - 1, fBuf) != NULL)
                {
                lineNo++;

                if (line[0] != '#')
                    {
                    continue;
                    }

                char *words[256];
                parse_it(words, line);

                MCICommandsE i;
                for (i = (MCICommandsE) 0; i < MCI_NUMCODES; i = (MCICommandsE) (i + 1))
                    {
                    if (SameString(words[0], mcikeywords[i]))
                        {
                        break;
                        }
                    }

                if (i != MCI_NUMCODES)
                    {
                    if (SameString(getmsg(627), words[1]) || SameString(getmsg(522), words[1]) ||
                            SameString(getmsg(316), words[1]) || SameString(getmsg(688), words[1]) ||
                            SameString(getmsg(690), words[1]))
                        {
                        tw()MCIRecursionChecker.SetEnabled(i, FALSE);
                        }
                    else if (SameString(getmsg(626), words[1]) || SameString(getmsg(521), words[1]) ||
                            SameString(getmsg(317), words[1]) || SameString(getmsg(687), words[1]) ||
                            SameString(getmsg(689), words[1]))
                        {
                        tw()MCIRecursionChecker.SetEnabled(i, TRUE);
                        }
                    else if (isNumeric(words[1]))
                        {
                        tw()MCIRecursionChecker.SetEnabled(i, !!atoi(words[1]));
                        }
                    else
                        {
                        tw()mPrintfCR(getcfgmsg(62), citfiles[C_MCI_CIT], mcikeywords[i], words[1]);
                        }
                    }
                else
                    {
                    tw()mPrintfCR(getcfgmsg(109), citfiles[C_MCI_CIT], ltoac(lineNo), words[0]);
                    }
                }

            discardData(d);
            }
        else
            {
            OutOfMemory(63);
            }

        fclose(fBuf);
        }
    else if (TellIfNotFound)
        {
        tw()mPrintfCR(getmsg(15), fullPathToMCICit);
        }
    }
