// --------------------------------------------------------------------------
// Citadel: NetNode.CPP
//
// Code for reading NODES.CIT.

#include "ctdl.h"
#pragma hdrstop

#include "log.h"
#include "group.h"
#include "net.h"
#include "hall.h"
#include "miscovl.h"


// --------------------------------------------------------------------------
// Contents
//
// shownode()       show NODES.CIT settings
// readnode()       read NODES.CIT to get the node's info for logbuf
// ReadNodesCit()   read NODES.CIT to get the node's info


#ifdef WINCIT
static const char *ltoac(int wow)
    {
    static label Buffer;
    sprintf(Buffer, "%d", wow);
    return (Buffer);
    }
#endif

// --------------------------------------------------------------------------
// Static Data

static discardable *nddd;
static int read_node_counter;


static Bool read_node_ddata(void)
    {
    read_node_counter++;

    if (!nddd)
        {
        if ((nddd = readData(10)) == NULL)
            {
            read_node_counter--;
            return (FALSE);
            }
        }

    return (TRUE);
    }

static void dump_node_ddata(void)
    {
    if (!(--read_node_counter))
        {
#ifndef WINCIT
        // Rather than implement a locking mechanism to ensure this is
        // never freed accidentally, we just leave it loaded all of the
        // time -- there's lots of memory on a Windows machine, right? :)

        discardData(nddd);
        nddd = NULL;
#endif
        }

    if (read_node_counter < 0)
        {
        read_node_counter = 0;
        }
    }

#define getnodemsg(x) nodemsgs[x]


// --------------------------------------------------------------------------
// shownode(): Show NODES.CIT settings.
//
// Input:
//  const NodesCitC *nd: The info to show.
//  Bool rmprint: TRUE to print to modem; FALSE to print to console only

void TERMWINDOWMEMBER shownode(const NodesCitC *nd, ModemConsoleE W)
    {
    assert (nd);

    if (!read_node_ddata())
        {
        wPrintf(W, getmsg(59));
        return;
        }

    const char **nettypes = (const char **) nddd->next->aux;
    const char **nodemsgs = (const char **) nddd->next->next->aux;

    wPrintf(W, getnodemsg(14), nd->GetName());                  wDoCR(W);
    wPrintf(W, getnodemsg(15), nd->GetOldRegion());             wDoCR(W);
    wPrintf(W, getnodemsg(16), nd->GetAlias(), nd->GetLocID()); wDoCR(W);
#ifdef WINCIT
    wPrintf(W, "IpAddress:   %s", nd->GetIpAddress());          wDoCR(W);
    wPrintf(W, "IpPort:      %d", nd->GetIpPort());             wDoCR(W);
#endif
    wPrintf(W, getnodemsg(17), nd->GetPreDial());               wDoCR(W);
    wPrintf(W, getnodemsg(18), nd->GetDialOut());               wDoCR(W);
    wPrintf(W, getnodemsg(19), bauds[nd->GetBaud()]);           wDoCR(W);
    wPrintf(W, getnodemsg(20), nd->GetLoginMacro());            wDoCR(W);
    wPrintf(W, getnodemsg(21), nd->GetDialTimeout());           wDoCR(W);
    wPrintf(W, getnodemsg(22), nd->GetProtocol());              wDoCR(W);
    wPrintf(W, getnodemsg(23), nd->GetWaitTimeout());           wDoCR(W);
    wPrintf(W, getnodemsg(24), nd->GetMailFileName());          wDoCR(W);

    label Buffer;
    wPrintf(W, getnodemsg(25), nd->GetAutoHall() > 0 ?
            HallData[nd->GetAutoHall()].GetName(Buffer, sizeof(Buffer)) : getnodemsg(28));
    wDoCR(W);

    wPrintf(W, getnodemsg(26), nd->GetAutoGroup() > 0 ?
            GroupData[nd->GetAutoGroup()].GetName(Buffer, sizeof(Buffer)) : getnodemsg(28));
    wDoCR(W);

    wPrintf(W, getnodemsg(27), GroupData[nd->GetMapUnknownGroup()].GetName(Buffer, sizeof(Buffer)));
    wDoCR(W);

    wPrintf(W, getnodemsg(29), nettypes[nd->GetNetworkType()],
            nd->GetNetworkRevisionNumber() ? nd->GetNetworkRevisionNumber() + '@' : ' ');
    wDoCR(W);

    wPrintf(W, getnodemsg(30), nd->GetCreatePacket());          wDoCR(W);
    wPrintf(W, getnodemsg(31), nd->GetExtractPacket());         wDoCR(W);
    wPrintf(W, getnodemsg(32), nd->IsAutoRoom());               wDoCR(W);
    wPrintf(W, getnodemsg(33), nd->GetRequest());               wDoCR(W);
    wPrintf(W, getnodemsg(34), nd->IsGateway());                wDoCR(W);
    wPrintf(W, getnodemsg(35), nd->GetFetch());                 wDoCR(W);
    wPrintf(W, getnodemsg(36), nd->GetNetFail());               wDoCR(W);
    wPrintf(W, getnodemsg(37), nd->GetRedial());                wDoCR(W);
    wPrintf(W, getnodemsg(38), nd->GetOutputPace());            wDoCR(W);

    dump_node_ddata();
    }

// --------------------------------------------------------------------------
// readnode(): Read NODES.CIT to get the node's info
//
// Return value:
//  TRUE: Read
//  FALSE: Not read

Bool TERMWINDOWMEMBER readnode(Bool Option)
    {
    label Buffer;
#ifdef WINCIT
    Bool toreturn = ReadNodesCit(this, &node, CurrentUser->GetName(Buffer, sizeof(Buffer)), CONSOLE, Option);
#else
    Bool toreturn = ReadNodesCit(&node, CurrentUser->GetName(Buffer, sizeof(Buffer)), CONSOLE, Option);
#endif

    if (debug && toreturn)
        {
        doccr();
        shownode(node, CONSOLE);
        doccr();
        }

    return (toreturn);
    }

static Bool clearNode(NodesCitC **node)
    {
    if (!*node)
        {
        // allocate space
        *node = new NodesCitC;

        if (!*node)
            {
            return (FALSE);
            }
        }

    (*node)->Clear();
    return (TRUE);
    }

void freeNode(NodesCitC **node)
    {
    if (node) 
        {
        delete *node;
        *node = NULL;
        }
    }


// --------------------------------------------------------------------------
// ReadNodesCit(): Read NODES.CIT to get the node's info
//
// Input:
//  const char *NameOrAddrToFind: The node to load
//  ModemConsoleE Warn: MODEM to display to modem; CONSOLE to display to console only; NOMODCON to display nowhere
//  Bool Option: TRUE if entry is optional; FALSE if not
//
// Output:
//  NodesCitC **newNode: Filled with node info. Allocated if NULL on entry

#ifdef WINCIT
Bool ReadNodesCit(TermWindowC *TW, NodesCitC **newNode, const char *NameOrAddrToFind, ModemConsoleE W, Bool option)
#else
Bool ReadNodesCit(NodesCitC **newNode, const char *NameOrAddrToFind, ModemConsoleE W, Bool option)
#endif
    {
    FILE *fBuf;
    char *words[256];
    int i;
    char path[80];
    int count;
    uint lineno = 0;
    l_slot logslot;

    if (!read_node_ddata())
        {
        TWwPrintf(W, getmsg(59));
        return (FALSE);
        }

    const char **nodekeywords = (const char **) nddd->aux;
    const char **nettypes = (const char **) nddd->next->aux;
    const char **nodemsgs = (const char **) nddd->next->next->aux;

    label NameToFind;
    CopyStringToBuffer(NameToFind, NameOrAddrToFind);

    if (SameString(NameOrAddrToFind, getnodemsg(2)))
        {
        // if default, start fresh
        if (!clearNode(newNode))
            {
            dump_node_ddata();
            return (FALSE);
            }
        }
    else
        {
        // else first read in default
#ifdef WINCIT
        ReadNodesCit(TW, newNode, getnodemsg(2), W, FALSE);
#else
        ReadNodesCit(newNode, getnodemsg(2), W, FALSE);
#endif

        if (*newNode)
            {
#ifdef WINCIT
            logslot = nodexists(NameOrAddrToFind, TW ? TW->LogOrder : NULL);
#else
            logslot = nodexists(NameOrAddrToFind);
#endif

            if (logslot == CERROR)
                {
                TWwDoCR(W);
                TWwPrintf(W, getnodemsg(3), NameOrAddrToFind);

                if (strlen(NameOrAddrToFind) < 4)
                    {
                    char temp[4];

                    CopyStringToBuffer(temp, NameOrAddrToFind);
                    strlwr(temp);
                    TWwPrintf(W, getnodemsg(4), temp, cfg.locID);
                    }

                dump_node_ddata();
                return (FALSE);
                }

            label Alias, LocID;

#ifdef WINCIT
            if (!*LogTab[TW ? TW->LogOrder[logslot] : logslot].GetAlias(Alias, sizeof(Alias)) ||
                    !*LogTab[TW ? TW->LogOrder[logslot] : logslot].GetLocID(LocID, sizeof(LocID)))
#else
            if (!*LTab(logslot).GetAlias(Alias, sizeof(Alias)) || !*LTab(logslot).GetLocID(LocID, sizeof(LocID)))
#endif
                {
                TWwDoCR(W);
                TWwPrintf(W, getnodemsg(5), NameOrAddrToFind);

                dump_node_ddata();
                return (FALSE);
                }

            (*newNode)->SetAlias(Alias);
            (*newNode)->SetLocID(LocID);

            label FileName;
            sprintf(FileName, getnodemsg(6), (*newNode)->GetAlias());
            (*newNode)->SetMailFileName(FileName);

            // Make sure NameToFind is a name, not address.
#ifdef WINCIT
            LogTab[TW ? TW->LogOrder[logslot] : logslot].GetName(NameToFind, sizeof(NameToFind));
#else
            LTab(logslot).GetName(NameToFind, sizeof(NameToFind));
#endif

            // Support for optional nodes.cit entries.
            (*newNode)->SetName(NameToFind);
            }
        }

    if (!*newNode)
        {
        cOutOfMemory(44);
        dump_node_ddata();
        return (FALSE);
        }

    compactMemory();

    sprintf(path, sbs, cfg.homepath, getnodemsg(8));

    if ((fBuf = fopen(path, FO_R)) == NULL) // ASCII mode
        {
        TWdoccr();
        TWcPrintf(getmsg(15), getnodemsg(8));
        TWdoccr();

        dump_node_ddata();
        return (option);
        }

    char line[256];
    Bool FoundOurEntry = FALSE;
    long pos = ftell(fBuf);
    while (fgets(line, 254, fBuf) != NULL)
        {
        lineno++;

        if (line[0] != '#')
            {
            pos = ftell(fBuf);
            continue;
            }

        // Don't read anything until we found our entry, except possible beginnings of our entry. (#NODE...)
        if (!FoundOurEntry && strnicmp(line + 1, nodekeywords[NOK_NODE], strlen(nodekeywords[NOK_NODE])) != SAMESTRING)
            {
            pos = ftell(fBuf);
            continue;
            }

        // Save a copy of the line in case this is the #LOGIN macro: parse_it
        // changes what is passed to it.
        char ltmp[256];
        CopyStringToBuffer(ltmp, line);

        count = parse_it(words, line);

        // Look up our first word in table of keywords.
        for (i = 0; i < NOK_NUM; i++)
            {
            if (SameString(words[0] + 1, nodekeywords[i]))
                {                   //^ add one for '#'
                break;
                }
            }

        if (i < NOK_NUM && !words[1][0])    // valid keywords need a param
            {
            TWcPrintf(getnodemsg(9), getnodemsg(8), ltoac(lineno), nodekeywords[i], ns);
            TWdoccr();
            continue;
            }

        if (i == NOK_NODE)
            {
            // This is a #NODE line... if we have alread found our entry,
            // then this is the start of the next one, and we are done.
            if (FoundOurEntry)
                {
                fclose(fBuf);
                dump_node_ddata();
                return (TRUE);
                }

            // This is a #NODE line... if it is for us, then we have now
            // found our entry. If not, then we continue looking.
            if (SameString(NameToFind, words[1]))
                {
                FoundOurEntry = TRUE;
                }
            else
                {
                pos = ftell(fBuf);
                continue;
                }
            }

        switch (i)
            {
            case NOK_BAUD:
                {
                PortSpeedE PS = digitbaud(atol(words[1]));

                if (PS == PS_ERROR)
                    {
                    TWcPrintf(getnodemsg(9), getnodemsg(8), ltoac(lineno), nodekeywords[i], words[1]);
                    TWdoccr();
                    }
                else
                    {
                    (*newNode)->SetBaud(PS);
                    }

                break;
                }

            case NOK_DIALOUT:
            case NOK_PHONE:
                {
                (*newNode)->SetDialOut(words[1]);

                if (strlen(words[1]) > 49)
                    {
                    TWcPrintf(getnodemsg(1), getnodemsg(8), ltoac(lineno), nodekeywords[i], 50);
                    TWdoccr();
                    }

                break;
                }

            case NOK_PREDIAL:
                {
                (*newNode)->SetPreDial(words[1]);

                if (strlen(words[1]) > 63)
                    {
                    TWcPrintf(getnodemsg(1), getnodemsg(8), ltoac(lineno), nodekeywords[i], 64);
                    TWdoccr();
                    }

                break;
                }

            case NOK_FETCH_TIMEOUT:
                {
                (*newNode)->SetFetchTimeout(atoi(words[1]));
                break;
                }

            case NOK_ZIP:
                {
                if (!words[2][0])   // need second param
                    {
                    TWcPrintf(getnodemsg(9), getnodemsg(8), ltoac(lineno), nodekeywords[i], ns);
                    TWdoccr();
                    continue;
                    }
                else
                    {
                    (*newNode)->SetCreatePacket(words[1]);

                    if (strlen(words[1]) > 39)
                        {
                        TWcPrintf(getnodemsg(1), getnodemsg(8), ltoac(lineno), nodekeywords[i], 40);
                        TWdoccr();
                        }

                    (*newNode)->SetExtractPacket(words[2]);

                    if (strlen(words[2]) > 39)
                        {
                        TWcPrintf(getnodemsg(1), getnodemsg(8), ltoac(lineno), nodekeywords[i], 40);
                        TWdoccr();
                        }
                    }

                break;
                }

            case NOK_NETWORK:
                {
                (*newNode)->SetNetworkRevisionNumber(0);

                NETTYPES j;
                for (j = (NETTYPES) 0; j < NET_NUM; j = (NETTYPES) (j + 1))
                    {
                    if (SameString(words[1], nettypes[j]))
                        {
                        break;
                        }
                    }

                if (j == NET_DCIT11)
                    {
                    j = NET_DCIT10;
                    }

                if (j == NET_6_9a)
                    {
                    (*newNode)->SetNetworkRevisionNumber(1);
                    j = NET_6_9;
                    }

                if (j == NET_NUM)
                    {
                    TWcPrintf(getnodemsg(9), getnodemsg(8), ltoac(lineno), nodekeywords[i], words[1]);
                    TWdoccr();
                    }
                else
                    {
                    (*newNode)->SetNetworkType(j);
                    }

                break;
                }

            case NOK_PROTOCOL:
                {
                (*newNode)->SetProtocol(words[1][0]);
                break;
                }

            case NOK_AUTOHALL:
                {
                (*newNode)->SetAutoHall(hallexists(words[1]));

                if (strlen(words[1]) > LABELSIZE)
                    {
                    TWcPrintf(getnodemsg(1), getnodemsg(8), ltoac(lineno), nodekeywords[i], LABELSIZE + 1);
                    TWdoccr();
                    }

                break;
                }

            case NOK_AUTOGROUP:
                {
                (*newNode)->SetAutoGroup(FindGroupByName(words[1]));

                if (strlen(words[1]) > LABELSIZE)
                    {
                    TWcPrintf(getnodemsg(1), getnodemsg(8), ltoac(lineno), nodekeywords[i], LABELSIZE + 1);
                    TWdoccr();
                    }

                break;
                }

            case NOK_MAPUNKGROUP:
                {
                g_slot GroupSlot = FindGroupByName(words[1]);

                if (GroupSlot == CERROR)
                    {
                    GroupSlot = SPECIALSECURITY;
                    }

                (*newNode)->SetMapUnknownGroup(GroupSlot);

                if (strlen(words[1]) > LABELSIZE)
                    {
                    TWcPrintf(getnodemsg(1), getnodemsg(8), ltoac(lineno), nodekeywords[i], LABELSIZE + 1);
                    TWdoccr();
                    }

                break;
                }

            case NOK_LOGIN:
                {
                (*newNode)->SetLoginMacro(ltmp + strlen(nodekeywords[NOK_LOGIN]) + 1);
                break;
                }

            case NOK_NODE:
                {
                // A bit silly.
                (*newNode)->SetName(words[1]);

                if (strlen(words[1]) > LABELSIZE)
                    {
                    TWcPrintf(getnodemsg(1), getnodemsg(8), ltoac(lineno), nodekeywords[i], LABELSIZE + 1);
                    TWdoccr();
                    }

                if (count > 2)
                    {
                    (*newNode)->SetOldRegion(words[2]);

                    if (strlen(words[2]) > LABELSIZE)
                        {
                        TWcPrintf(getnodemsg(1), getnodemsg(8), ltoac(lineno), nodekeywords[i], LABELSIZE + 1);
                        TWdoccr();
                        }
                    }
                else
                    {
                    (*newNode)->SetOldRegion(ns);
                    }

                (*newNode)->FreeMappedGroups();
                (*newNode)->SetRoomOffset(0);

                break;
                }

            case NOK_DIAL_TIMEOUT:
                {
                (*newNode)->SetDialTimeout(atoi(words[1]));
                break;
                }

            case NOK_REQUEST:
                {
                (*newNode)->SetRequest(atoi(words[1]));
                break;
                }

            case NOK_WAIT_TIMEOUT:
                {
                (*newNode)->SetWaitTimeout(atoi(words[1]));
                break;
                }

            case NOK_AUTOROOM:
                {
                (*newNode)->SetAutoRoom(atoi(words[1]));
                break;
                }

            case NOK_VERBOSE:
                {
                (*newNode)->SetDefaultVerbose();

                const char **verbosekeywords = (const char **) nddd->next->next->next->aux;

                for (int j = 1; j < count; j++)
                    {
                    if (SameString(words[j], getnodemsg(39)))
                        {
                        (*newNode)->SetDefaultVerbose();
                        }
                    else if (SameString(words[j], getnodemsg(40)))
                        {
                        (*newNode)->SetDefaultVerbose();
                        (*newNode)->SetVerbose(NCV_FILE69INFULL, TRUE);
                        (*newNode)->SetVerbose(NCV_NOACCESS, TRUE);
                        }
                    else if (SameString(words[j], getnodemsg(41)))
                        {
                        (*newNode)->SetDefaultVerbose();
                        (*newNode)->SetVerbose(NCV_FILE69INFULL, TRUE);
                        (*newNode)->SetVerbose(NCV_NOACCESS, TRUE);
                        (*newNode)->SetVerbose(NCV_ROOMCREATED, TRUE);
                        (*newNode)->SetVerbose(NCV_ROOMNOTCREATED, TRUE);
                        (*newNode)->SetVerbose(NCV_NETIDNOTFOUND, TRUE);
                        (*newNode)->SetVerbose(NCV_NONETIDONSYSTEM, TRUE);
                        }
                    else
                        {
                        const char *Keyword;
                        Bool NewSetting;

                        if (words[j][0] == '!')
                            {
                            Keyword = words[j] + 1;
                            NewSetting = FALSE;
                            }
                        else
                            {
                            Keyword = words[j];
                            NewSetting = TRUE;
                            }

                        NCV_Type TestType;

                        for (TestType = (NCV_Type) 0; TestType < NCV_MAX; TestType = (NCV_Type) (TestType + 1))
                            {
                            if (SameString(Keyword, verbosekeywords[TestType]))
                                {
                                (*newNode)->SetVerbose(TestType, NewSetting);
                                break;
                                }
                            }

                        if (TestType == NCV_MAX)
                            {
                            if (SameString(Keyword, getnodemsg(13)))
                                {
                                // ALL
                                for (TestType = (NCV_Type) 0; TestType < NCV_MAX; TestType = (NCV_Type) (TestType + 1))
                                    {
                                    (*newNode)->SetVerbose(TestType, NewSetting);
                                    }
                                }
                            else
                                {
                                TWcPrintf(getnodemsg(12), getnodemsg(8), ltoac(lineno), words[0], Keyword);
                                TWdoccr();
                                }
                            }
                        }
                    }

                break;
                }

            case NOK_CHAT:
                {
                (*newNode)->SetDefaultChat();

                const char **chatkeywords = (const char **) nddd->next->next->next->next->aux;

                for (int j = 1; j < count; j++)
                    {
                    const char *Keyword;
                    Bool NewSetting;

                    if (words[j][0] == '!')
                        {
                        Keyword = words[j] + 1;
                        NewSetting = FALSE;
                        }
                    else
                        {
                        Keyword = words[j];
                        NewSetting = TRUE;
                        }

                    NCC_Type TestType;

                    for (TestType = (NCC_Type) 0; TestType < NCC_MAX; TestType = (NCC_Type) (TestType + 1))
                        {
                        if (SameString(Keyword, chatkeywords[TestType]))
                            {
                            (*newNode)->SetChat(TestType, NewSetting);
                            break;
                            }
                        }

                    if (TestType == NCV_MAX)
                        {
                        if (SameString(Keyword, getnodemsg(13)))
                            {
                            // ALL
                            for (TestType = (NCC_Type) 0; TestType < NCC_MAX; TestType = (NCC_Type) (TestType + 1))
                                {
                                (*newNode)->SetChat(TestType, NewSetting);
                                }
                            }
                        else
                            {
                            TWcPrintf(getnodemsg(12), getnodemsg(8), ltoac(lineno), words[0], Keyword);
                            TWdoccr();
                            }
                        }
                    }

                break;
                }

            case NOK_REDIAL:
                {
                (*newNode)->SetRedial(atoi(words[1]));
                break;
                }

            case NOK_ROOM:
                {
                (*newNode)->SetRoomOffset(pos);
                fclose(fBuf);
                dump_node_ddata();
                return (TRUE);
                }

            case NOK_GROUP:
                {
                if (!(*newNode)->AddMappedGroups(words[1], words[2]))
                    {
                    TWcPrintf(getnodemsg(10), words[1]);
                    TWdoccr();
                    }

                if (strlen(words[1]) > LABELSIZE)
                    {
                    TWcPrintf(getnodemsg(1), getnodemsg(8), ltoac(lineno), nodekeywords[i], LABELSIZE + 1);
                    TWdoccr();
                    }

                if (strlen(words[2]) > LABELSIZE)
                    {
                    TWcPrintf(getnodemsg(1), getnodemsg(8), ltoac(lineno), nodekeywords[i], LABELSIZE + 1);
                    TWdoccr();
                    }

                break;
                }

            case NOK_GATEWAY:
                {
                (*newNode)->SetGateway(atoi(words[1]));
                break;
                }

            case NOK_FETCH:
                {
                (*newNode)->SetFetch(atoi(words[1]));
                break;
                }

            case NOK_NETFAIL:
                {
                (*newNode)->SetNetFail(atoi(words[1]));
                break;
                }

            case NOK_OUTPUTPACE:
                {
                (*newNode)->SetOutputPace(atoi(words[1]));
                break;
                }

            case NOK_IPADDRESS:
                {
#ifdef WINCIT
                (*newNode)->SetIpAddress(words[1]);

                if (strlen(words[1]) > 255)
                    {
                    TWcPrintf(getnodemsg(1), getnodemsg(8), ltoac(lineno), nodekeywords[i], 256);
                    TWdoccr();
                    }
#endif
                break;
                }

            case NOK_IPPORT:
                {
#ifdef WINCIT
                (*newNode)->SetIpPort(atoi(words[1]));
#endif
                break;
                }

            default:
                {
                TWcPrintf(getnodemsg(11), getnodemsg(8), ltoac(lineno), words[0]);
                TWdoccr();

                break;
                }
            }

        pos = ftell(fBuf);
        }

    fclose(fBuf);

    if (!FoundOurEntry && !option && debug)
        {
        TWwDoCR(W);
        TWwPrintf(W, getmsg(206), NameOrAddrToFind);
        TWwDoCR(W);
        }

    dump_node_ddata();

    if (option)
        {
        return (TRUE);
        }
    else
        {
        return (FoundOurEntry);
        }
    }
