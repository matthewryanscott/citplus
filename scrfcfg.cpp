// --------------------------------------------------------------------------
// Citadel: ScrFCfg.CPP
//
// Script Functions dealing with the system's Configuration.

#include "ctdl.h"
#pragma hdrstop

#include "scrfunc.h"
#include "miscovl.h"
#include "msg.h"

///////////////////////////// CONFIG STUFF \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\

void sfGetmaxhalls(strList *)
	{
	SETINT(cfg.maxhalls);
	}

void sfGetmaxgroups(strList *)
	{
	SETINT(cfg.maxgroups);
	}

void sfGetmaxrooms(strList *)
	{
	SETINT(cfg.maxrooms);
	}

void sfGetmaxlogtab(strList *)
	{
	SETINT(cfg.MAXLOGTAB);
	}

void sfGetnmessages(strList *)
	{
	SETLONG((long) cfg.nmessages);
	}

void sfGetcallno(strList *)
	{
	SETLONG(cfg.callno);
	}

void sfGetoldest(strList *)
	{
	SETLONG(MessageDat.OldestMessageInFile());
	}

void sfGetnewest(strList *)
	{
	SETLONG(MessageDat.NewestMessage());
	}

void sfGetmtoldest(strList *)
	{
	SETLONG(MessageDat.OldestMessageInTable());
	}

void sfGetnodename(strList *)
	{
	SETSTR(cfg.nodeTitle);
	}

void sfGetsysopname(strList *)
	{
	SETSTR(cfg.sysop);
	}

void sfGetforwardsys(strList *)
	{
	SETBOOL(cfg.forward);
	}

void sfGetnoderegion(strList *)
	{
	SETSTR(cfg.nodeRegion);
	}

void sfGetnodecountry(strList *)
	{
	SETSTR(cfg.nodeCountry);
	}

void sfGetCit86Country(strList *)
	{
	SETSTR(cfg.Cit86Country);
	}

void sfGetCit86Domain(strList *)
	{
	SETSTR(cfg.Cit86Domain);
	}

void sfGetnodesig(strList *)
	{
	SETSTR(cfg.nodeSignature);
	}

void sfGettwitregion(strList *)
	{
	SETSTR(cfg.twitRegion);
	}

void sfGettwitcountry(strList *)
	{
	SETSTR(cfg.twitCountry);
	}

void sfGetalias(strList *)
	{
	SETSTR(cfg.alias);
	}

void sfGetLocID(strList *)
	{
	SETSTR(cfg.locID);
	}

void sfGetnodephone(strList *)
	{
	SETSTR(cfg.nodephone);
	}

void sfGetmdata(strList *)
	{
	SETLONG(cfg.mdata);
	}

void sfGetdumbmodem(strList *)
	{
	SETLONG(cfg.dumbmodem);
	}

void sfGetinitbaud(strList *)
	{
	SETLONG(bauds[cfg.initbaud]);
	}

void sfGetminbaud(strList *)
	{
	SETLONG(connectbauds[cfg.minbaud]);
	}

void sfGetmodsetup(strList *)
	{
	SETSTR(cfg.modsetup);
	}

void sfGetmodunsetup(strList *)
	{
	SETSTR(cfg.modunsetup);
	}

void sfGetdownshift(strList *)
	{
	SETSTR(cfg.downshift);
	}

void sfGetdialsetup(strList *)
	{
	SETSTR(cfg.dialsetup);
	}

void sfGetdialpref(strList *)
	{
	SETSTR(cfg.dialpref);
	}

void sfGetdialring(strList *)
	{
	SETSTR(cfg.dialring);
	}

void sfGetupday(strList *params)
	{
	SETBOOL(cfg.updays[evaluateInt(params->string)]);
	}

void sfGetuphour(strList *params)
	{
	SETBOOL(cfg.uphours[evaluateInt(params->string)]);
	}

void sfGethangupdelay(strList *)
	{
	SETLONG(cfg.hangupdelay);
	}

void sfGethangup(strList *)
	{
	SETSTR(cfg.hangup);
	}

void sfGetoffhookstr(strList *)
	{
	SETSTR(cfg.offhookstr);
	}

void sfGethomepath(strList *)
	{
	SETSTR(cfg.homepath);
	}

void sfGetmsgpath(strList *)
	{
	SETSTR(cfg.msgpath);
	}

void sfGethelppath(strList *)
	{
	SETSTR(cfg.helppath);
	}

void sfGethelppath2(strList *)
	{
	SETSTR(cfg.helppath2);
	}

void sfGettemppath(strList *)
	{
#ifdef WINCIT
	SETSTR(pCurScript->TW ? pCurScript->TW->LocalTempPath : cfg.temppath);
#else
	SETSTR(cfg.temppath);
#endif
	}

void sfGetroompath(strList *)
	{
	SETSTR(cfg.roompath);
	}

void sfGettrapfile(strList *)
	{
	SETSTR(cfg.trapfile);
	}

void sfGetapplpath(strList *)
	{
	SETSTR(cfg.aplpath);
	}

void sfGetdirpath(strList *)
	{
	SETSTR(cfg.dirpath);
	}

void sfGettranspath(strList *)
	{
	SETSTR(cfg.transpath);
	}

void sfGetprinter(strList *)
	{
	SETSTR(cfg.printer);
	}

void sfGetdlpath(strList *)
	{
	SETSTR(cfg.dlpath);
	}

void sfGetlogextdir(strList *)
	{
	SETSTR(cfg.logextdir);
	}

void sfGetautoansi(strList *)
	{
	SETLONG(cfg.autoansi);
	}

void sfGetentername(strList *)
	{
	SETSTR(cfg.enter_name);
	}

void sfGetnewuserapp(strList *)
	{
	SETSTR(cfg.newuserapp);
	}

void sfGetoldcount(strList *)
	{
	SETLONG(cfg.oldcount);
	}

void sfGetlclosedsys(strList *)
	{
	SETBOOL(cfg.l_closedsys);
	}

void sfGetlverified(strList *)
	{
	SETBOOL(cfg.l_verified);
	}

void sfGetlquestionnaire(strList *)
	{
	SETBOOL(cfg.NUQuest[0] != NUQ_NUMQUEST);
	}

void sfGetlsysmsg(strList *)
	{
	SETBOOL(cfg.l_sysop_msg);
	}

void sfGetlcreate(strList *)
	{
	SETBOOL(cfg.l_create);
	}

void sfGetunlogtimeout(strList *)
	{
	SETLONG(cfg.unlogtimeout);
	}

void sfGettimeout(strList *)
	{
	SETLONG(cfg.timeout);
	}

void sfGetunlogbal(strList *)
	{
	SETLONG(cfg.unlogbal);
	}

void sfGetdefdatestamp(strList *)
	{
	SETSTR(cfg.datestamp);
	}

void sfGetdefvdatestamp(strList *)
	{
	SETSTR(cfg.vdatestamp);
	}

void sfGetdefrprompt(strList *)
	{
	SETSTR(cfg.prompt);
	}

void sfGetmsgverb(strList *)
	{
	SETSTR(cfg.msg_done);
	}

void sfGetborder(strList *params)
	{
	SETSTR(&(borders[81 * evaluateInt(params->string)]));
	}

void sfGetmaxborders(strList *)
	{
	SETLONG(cfg.maxborders);
	}

void sfGetdefnetprefix(strList *)
	{
	SETSTR(cfg.netPrefix);
	}

void sfGetnopwecho(strList *)
	{
	SETLONG(cfg.nopwecho);
	}

void sfGetanonauthor(strList *)
	{
	SETSTR(cfg.anonauthor);
	}

void sfGettwirly(strList *)
	{
	SETSTR(cfg.twirly);
	}

void sfGettwirlypause(strList *)
	{
	SETLONG(cfg.twirlypause);
	}

void sfGetmessagek(strList *)
	{
	SETLONG(cfg.MsgDatSizeInK);
	}

void sfGetmaxfiles(strList *)
	{
	SETLONG(cfg.maxfiles);
	}

void sfGetsetmsgnym(strList *)
	{
	SETBOOL(cfg.msgNym);
	}

void sfGetsysborders(strList *)
	{
	SETBOOL(cfg.borders);
	}

void sfGetforcelogin(strList *)
	{
	SETBOOL(cfg.forcelogin);
	}

void sfGetoffhook(strList *)
	{
	SETLONG(cfg.offhook);
	}

void sfGetaccounting(strList *)
	{
	SETBOOL(cfg.accounting);
	}

void sfGetnetmail(strList *)
	{
	SETBOOL(cfg.netmail);
	}

void sfGetsubhubs(strList *)
	{
	SETLONG(cfg.subhubs);
	}

void sfGetcolors(strList *)
	{
	SETBOOL(cfg.colors);
	}

void sfGetmoreprompt(strList *)
	{
	SETSTR(cfg.moreprompt);
	}

void sfGetsleepprompt(strList *)
	{
	SETSTR(cfg.sleepprompt);
	}

void sfGetsetcensor(strList *)
	{
	SETBOOL(cfg.censor);
	}

void sfGetcountbeep(strList *)
	{
	SETBOOL(cfg.countbeep);
	}

void sfGetsleepcount(strList *)
	{
	SETLONG(cfg.sleepcount);
	}

void sfGetdialmacro(strList *params)
	{
	SETSTR(cfg.dialmacro[evaluateInt(params->string)]);
	}

void sfGetdiskfree(strList *)
	{
	SETLONG(cfg.diskfree);
	}

void sfGetaidechathour(strList *params)
	{
	SETBOOL(cfg.aideChatHours[evaluateInt(params->string)]);
	}

void sfGetentersur(strList *)
	{
	SETBOOL(cfg.entersur);
	}

void sfGetaidehall(strList *)
	{
	SETBOOL(cfg.aidehall);
	}

void sfGetreadluser(strList *)
	{
	SETBOOL(cfg.readluser);
	}

void sfGetenterok(strList *)
	{
	SETBOOL(cfg.unlogEnterOk);
	}

void sfGetreadok(strList *)
	{
	SETBOOL(cfg.unlogReadOk);
	}

void sfGetroomok(strList *)
	{
	SETBOOL(cfg.nonAideRoomOk);
	}

void sfGetaidemoderate(strList *)
	{
	SETBOOL(cfg.moderate);
	}

void sfGetsurnames(strList *)
	{
	SETBOOL(cfg.surnames);
	}

void sfGetnetsurnames(strList *)
	{
	SETBOOL(cfg.netsurname);
	}

void sfGettitles(strList *)
	{
	SETBOOL(cfg.titles);
	}

void sfGetnettitles(strList *)
	{
	SETBOOL(cfg.nettitles);
	}

void sfGetbios(strList *)
	{
	SETBOOL(cfg.bios);
	}

void sfGetattr(strList *)
	{
	SETINT(cfg.attr);
	}

void sfGetwattr(strList *)
	{
	SETINT(cfg.wattr);
	}

void sfGetcattr(strList *)
	{
	SETINT(cfg.cattr);
	}

void sfGetbattr(strList *)
	{
	SETINT(cfg.battr);
	}

void sfGetuttr(strList *)
	{
	SETINT(cfg.uttr);
	}

void sfGetscreensave(strList *params)
	{
	switch (evaluateInt(params->string))
		{
		case 1:	SETLONG(cfg.screensave);					break;
		case 2:	SETBOOL(cfg.fucking_stupid);				break;
		case 3: SETBOOL(cfg.really_fucking_stupid);			break;
		case 4: SETLONG(cfg.turboClock);                    break;
		case 5: SETBOOL(cfg.really_really_fucking_stupid);  break;
		}
	}

void sfGetconnectwait(strList *)
	{
	SETLONG(cfg.connectwait);
	}

void sfGetidletimeout(strList *)
	{
	SETLONG(cfg.idle);
	}

void sfGetf6pass(strList *)
	{
	SETSTR(cfg.f6pass);
	}

void sfGetmessageroom(strList *)
	{
	SETLONG(cfg.MessageRoom);
	}

void sfGetchaton(strList *)
	{
	SETBOOL(!cfg.noChat);
	}

void sfGetbells(strList *)
	{
	SETLONG(cfg.noBells);
	}

void sfGetmcion(strList *)
	{
	SETBOOL(cfg.mci);
	}

void sfGetmciname(strList *)
	{
	SETSTR(cfg.mci_name);
	}

void sfGetmcifirstname(strList *)
	{
	SETSTR(cfg.mci_firstname);
	}

void sfGetmcitime(strList *)
	{
	SETSTR(cfg.mci_time);
	}

void sfGetmcidate(strList *)
	{
	SETSTR(cfg.mci_date);
	}

void sfGetmcipoop(strList *)
	{
	SETSTR(cfg.mci_poop);
	}

void sfGetvmemfile(strList *)
	{
#ifdef AUXMEM
	SETSTR(cfg.vmemfile);
#else
	SETSTR(ns);
#endif
	}

void sfGetcheckcts(strList *)
	{
	SETBOOL(cfg.checkCTS);
	}

void sfGetpoop(strList *)
	{
	SETLONG(cfg.poop);
	}

void sfGetmaxpoop(strList *)
	{
	SETLONG(cfg.maxpoop);
	}

void sfGetpoopuser(strList *)
	{
	SETSTR(cfg.poopuser);
	}

void sfGetprinterprompt(strList *)
	{
	SETBOOL(cfg.printerprompt);
	}

void sfGetscrollsize(strList *)
	{
	SETLONG(cfg.scrollSize);
	}

void sfGetscrollcolors(strList *)
	{
	SETBOOL(cfg.scrollColors);
	}

void sfGetscrolltimeout(strList *)
	{
	SETLONG(cfg.scrollTimeOut);
	}

void sfGetmsgcompress(strList *)
	{
	SETBOOL(cfg.msgCompress);
	}

void sfGetrestoremode(strList *)
	{
	SETBOOL(cfg.restore_mode);
	}

void sfGetexpire(strList *)
	{
	SETLONG(cfg.expire);
	}

void sfGetmaxjumpback(strList *)
	{
	SETLONG(cfg.maxjumpback - 1);
	}

void sfGetfuelbarempty(strList *)
	{
	SETINT(cfg.fuelbarempty);
	}

void sfGetfuelbarfull(strList *)
	{
	SETINT(cfg.fuelbarfull);
	}

void sfGetstatnum(strList *)
	{
	SETLONG(cfg.statnum);
	}

void sfSetidletimeout(strList *params)
	{
	cfg.idle = evaluateInt(params->string);
	}

void sfSetnodename(strList *params)
	{
	CopyStringToBuffer(cfg.nodeTitle, evaluateString(params->string));
	}

void sfSetsysopname(strList *params)
	{
	CopyStringToBuffer(cfg.sysop, evaluateString(params->string));
	}

void sfSetforwardsys(strList *params)
	{
	cfg.forward = evaluateBool(params->string);
	}

void sfSetnoderegion(strList *params)
	{
	CopyStringToBuffer(cfg.nodeRegion, evaluateString(params->string));
	}

void sfSetnodecountry(strList *params)
	{
	CopyStringToBuffer(cfg.nodeCountry, evaluateString(params->string));
	}

void sfSetCit86Country(strList *params)
	{
	CopyStringToBuffer(cfg.Cit86Country, evaluateString(params->string));
	}

void sfSetCit86Domain(strList *params)
	{
	CopyStringToBuffer(cfg.Cit86Domain, evaluateString(params->string));
	}

void sfSettwitregion(strList *params)
	{
	CopyStringToBuffer(cfg.twitRegion, evaluateString(params->string));
	}

void sfSettwitcountry(strList *params)
	{
	CopyStringToBuffer(cfg.twitCountry, evaluateString(params->string));
	}

void sfSetnodephone(strList *params)
	{
	CopyStringToBuffer(cfg.nodephone, evaluateString(params->string));
	}

void sfSetmdata(strList *params)
	{
	cfg.mdata = evaluateInt(params->string);
	}

void sfSetdumbmodem(strList *params)
	{
	cfg.dumbmodem = evaluateInt(params->string);
	}

void sfSetinitbaud(strList *params)
	{
	PortSpeedE PS = digitbaud(evaluateLong(params->string));

	if (PS != PS_ERROR)
		{
		cfg.initbaud = PS;
		SETBOOL(TRUE);
		}
	else
		{
		SETBOOL(FALSE);
		}
	}

void sfSetminbaud(strList *params)
	{
	ModemSpeedE MS = digitConnectbaud(evaluateLong(params->string));

	if (MS != MS_ERROR)
		{
		cfg.minbaud = MS;
		SETBOOL(TRUE);
		}
	else
		{
		SETBOOL(FALSE);
		}
	}

void sfSetmodsetup(strList *params)
	{
	CopyStringToBuffer(cfg.modsetup, evaluateString(params->string));
	}

void sfSetmodunsetup(strList *params)
	{
	CopyStringToBuffer(cfg.modunsetup, evaluateString(params->string));
	}

void sfSetdownshift(strList *params)
	{
	CopyStringToBuffer(cfg.downshift, evaluateString(params->string));
	}

void sfSetdialsetup(strList *params)
	{
	CopyStringToBuffer(cfg.dialsetup, evaluateString(params->string));
	}

void sfSetdialpref(strList *params)
	{
	CopyStringToBuffer(cfg.dialpref, evaluateString(params->string));
	}

void sfSetdialring(strList *params)
	{
	CopyStringToBuffer(cfg.dialring, evaluateString(params->string));
	}

void sfSetupday(strList *params)
	{
	cfg.updays[evaluateInt(params->string)] = (char) evaluateBool(params->next->string);
	}

void sfSetuphour(strList *params)
	{
	cfg.uphours[evaluateInt(params->string)] = (char) evaluateBool(params->next->string);
	}

void sfSethangupdelay(strList *params)
	{
	cfg.hangupdelay = evaluateInt(params->string);
	}

void sfSethangup(strList *params)
	{
	CopyStringToBuffer(cfg.hangup, evaluateString(params->string));
	}

void sfSetoffhookstr(strList *params)
	{
	CopyStringToBuffer(cfg.offhookstr, evaluateString(params->string));
	}

void sfSethomepath(strList *params)
	{
	CopyStringToBuffer(cfg.homepath, evaluateString(params->string));
	}

void sfSetmsgpath(strList *params)
	{
	CopyStringToBuffer(cfg.msgpath, evaluateString(params->string));
	}

void sfSethelppath(strList *params)
	{
	CopyStringToBuffer(cfg.helppath, evaluateString(params->string));
	}

void sfSethelppath2(strList *params)
	{
	CopyStringToBuffer(cfg.helppath2, evaluateString(params->string));
	}

void sfSettemppath(strList *params)
	{
#ifdef WINCIT
	if (pCurScript->TW)
		{
		CopyStringToBuffer(pCurScript->TW->LocalTempPath, evaluateString(params->string));
		}
	else
		{
		CopyStringToBuffer(cfg.temppath, evaluateString(params->string));
		}
#else
	CopyStringToBuffer(cfg.temppath, evaluateString(params->string));
#endif
	}

void sfSetroompath(strList *params)
	{
	CopyStringToBuffer(cfg.roompath, evaluateString(params->string));
	}

void sfSettrapfile(strList *params)
	{
	CopyStringToBuffer(cfg.trapfile, evaluateString(params->string));
	}

void sfSetapplpath(strList *params)
	{
	CopyStringToBuffer(cfg.aplpath, evaluateString(params->string));
	}

void sfSetdirpath(strList *params)
	{
	CopyStringToBuffer(cfg.dirpath, evaluateString(params->string));
	}

void sfSettranspath(strList *params)
	{
	CopyStringToBuffer(cfg.transpath, evaluateString(params->string));
	}

void sfSetprinter(strList *params)
	{
	CopyStringToBuffer(cfg.printer, evaluateString(params->string));
	}

void sfSetdlpath(strList *params)
	{
	CopyStringToBuffer(cfg.dlpath, evaluateString(params->string));
	}

void sfSetlogextdir(strList *params)
	{
	CopyStringToBuffer(cfg.logextdir, evaluateString(params->string));
	}

void sfSetautoansi(strList *params)
	{
	cfg.autoansi = evaluateInt(params->string);
	}

void sfSetentername(strList *params)
	{
	CopyStringToBuffer(cfg.enter_name, evaluateString(params->string));
	}

void sfSetnewuserapp(strList *params)
	{
	CopyStringToBuffer(cfg.newuserapp, evaluateString(params->string));
	}

void sfSetoldcount(strList *params)
	{
	cfg.oldcount = evaluateInt(params->string);
	}

void sfSetlclosedsys(strList *params)
	{
	cfg.l_closedsys = evaluateBool(params->string);
	}

void sfSetlverified(strList *params)
	{
	cfg.l_verified = evaluateBool(params->string);
	}

void sfSetlsysmsg(strList *params)
	{
	cfg.l_sysop_msg = evaluateBool(params->string);
	}

void sfSetlcreate(strList *params)
	{
	cfg.l_create = evaluateBool(params->string);
	}

void sfSetunlogtimeout(strList *params)
	{
	cfg.unlogtimeout = evaluateInt(params->string);
	}

void sfSettimeout(strList *params)
	{
	cfg.timeout = evaluateInt(params->string);
	}

void sfSetunlogbal(strList *params)
	{
	cfg.unlogbal = evaluateLong(params->string);
	}

void sfSetdefdatestamp(strList *params)
	{
	CopyStringToBuffer(cfg.datestamp, evaluateString(params->string));
	}

void sfSetdefvdatestamp(strList *params)
	{
	CopyStringToBuffer(cfg.vdatestamp, evaluateString(params->string));
	}

void sfSetdefrprompt(strList *params)
	{
	CopyStringToBuffer(cfg.prompt, evaluateString(params->string));
	}

void sfSetmsgnym(strList *params)
	{
	label L;

	CopyStringToBuffer(cfg.Lmsg_nym, MakeItLower(evaluateString(params->string), L));
	CopyStringToBuffer(cfg.Umsg_nym, MakeItUpper(evaluateString(params->string), L));
	}

void sfSetmsgsnym(strList *params)
	{
	label L;

	CopyStringToBuffer(cfg.Lmsgs_nym, MakeItLower(evaluateString(params->string), L));
	CopyStringToBuffer(cfg.Umsgs_nym, MakeItUpper(evaluateString(params->string), L));
	}

void sfSetcreditnym(strList *params)
	{
	label L;

	CopyStringToBuffer(cfg.Lcredit_nym, MakeItLower(evaluateString(params->string), L));
	CopyStringToBuffer(cfg.Ucredit_nym, MakeItUpper(evaluateString(params->string), L));
	}

void sfSetcreditsnym(strList *params)
	{
	label L;

	CopyStringToBuffer(cfg.Lcredits_nym, MakeItLower(evaluateString(params->string), L));
	CopyStringToBuffer(cfg.Ucredits_nym, MakeItUpper(evaluateString(params->string), L));
	}

void sfSetmsgverb(strList *params)
	{
	CopyStringToBuffer(cfg.msg_done, evaluateString(params->string));
	}

void sfSetborder(strList *params)
	{
	CopyString2Buffer(&(borders[81 * evaluateInt(params->string)]),  evaluateString(params->next->string), BORDERSIZE + 1);
	}

void sfSetdefnetprefix(strList *params)
	{
	CopyStringToBuffer(cfg.netPrefix, evaluateString(params->string));
	}

void sfSetnopwecho(strList *params)
	{
	cfg.nopwecho = evaluateInt(params->string);
	}

void sfSetanonauthor(strList *params)
	{
	CopyStringToBuffer(cfg.anonauthor, evaluateString(params->string));
	}

void sfSettwirly(strList *params)
	{
	CopyStringToBuffer(cfg.twirly, evaluateString(params->string));
	}

void sfSettwirlypause(strList *params)
	{
	cfg.twirlypause = evaluateInt(params->string);
	}

void sfSetmaxfiles(strList *params)
	{
	cfg.maxfiles = evaluateInt(params->string);
	}

void sfSetsetmsgnym(strList *params)
	{
	cfg.msgNym = evaluateBool(params->string);
	}

void sfSetforcelogin(strList *params)
	{
	cfg.forcelogin = evaluateBool(params->string);
	}

void sfSetoffhook(strList *params)
	{
	cfg.offhook = evaluateInt(params->string);
	}

void sfSetnetmail(strList *params)
	{
	cfg.netmail = evaluateBool(params->string);
	}

void sfSetsubhubs(strList *params)
	{
	cfg.subhubs = evaluateInt(params->string);
	}

void sfSetcolors(strList *params)
	{
	cfg.colors = evaluateBool(params->string);
	}

void sfSetmoreprompt(strList *params)
	{
	CopyStringToBuffer(cfg.moreprompt, evaluateString(params->string));
	}

void sfSetsleepprompt(strList *params)
	{
	CopyStringToBuffer(cfg.sleepprompt, evaluateString(params->string));
	}

void sfSetsetcensor(strList *params)
	{
	cfg.censor = evaluateBool(params->string);
	}

void sfSetcountbeep(strList *params)
	{
	cfg.countbeep = evaluateBool(params->string);
	}

void sfSetsleepcount(strList *params)
	{
	cfg.sleepcount = evaluateInt(params->string);
	}

void sfSetdialmacro(strList *params)
	{
	CopyString2Buffer(cfg.dialmacro[evaluateInt(params->string)], evaluateString(params->next->string), 80);
	}

void sfSetdiskfree(strList *params)
	{
	cfg.diskfree = evaluateLong(params->string);
	}

void sfSetaidechathour(strList *params)
	{
	cfg.aideChatHours[evaluateInt(params->string)] = evaluateBool(params->next->string);
	}

void sfSetentersur(strList *params)
	{
	cfg.entersur = evaluateBool(params->string);
	}

void sfSetaidehall(strList *params)
	{
	cfg.aidehall = evaluateBool(params->string);
	}

void sfSetreadluser(strList *params)
	{
	cfg.readluser = evaluateBool(params->string);
	}

void sfSetenterok(strList *params)
	{
	cfg.unlogEnterOk = evaluateBool(params->string);
	}

void sfSetreadok(strList *params)
	{
	cfg.unlogReadOk = evaluateBool(params->string);
	}

void sfSetroomok(strList *params)
	{
	cfg.nonAideRoomOk = evaluateBool(params->string);
	}

void sfSetaidemoderate(strList *params)
	{
	cfg.moderate = evaluateBool(params->string);
	}

void sfSetsurnames(strList *params)
	{
	cfg.surnames = evaluateBool(params->string);
	}

void sfSetnetsurnames(strList *params)
	{
	cfg.netsurname = evaluateBool(params->string);
	}

void sfSettitles(strList *params)
	{
	cfg.titles = evaluateBool(params->string);
	}

void sfSetnettitles(strList *params)
	{
	cfg.nettitles = evaluateBool(params->string);
	}

void sfSetattr(strList *params)
	{
	cfg.attr = (char) evaluateInt(params->string);
	}

void sfSetwattr(strList *params)
	{
	cfg.wattr = (char) evaluateInt(params->string);
	}

void sfSetcattr(strList *params)
	{
	cfg.cattr = (char) evaluateInt(params->string);
	}

void sfSetbattr(strList *params)
	{
	cfg.battr = (char) evaluateInt(params->string);
	}

void sfSetuttr(strList *params)
	{
	cfg.uttr = (char) evaluateInt(params->string);
	}

void sfSetscreensave(strList *params)
	{
	switch (evaluateInt(params->string))
		{
		case 1:
			{
			cfg.screensave = evaluateInt(params->next->string);
			break;
			}

		case 2:
			{
			cfg.fucking_stupid = evaluateBool(params->next->string);
			break;
			}

		case 3:
			{
			cfg.really_fucking_stupid = evaluateBool(params->next->string);
			break;
			}

		case 4:
			{
			cfg.turboClock = evaluateInt(params->next->string);
			break;
			}

		case 5:
			{
			cfg.really_really_fucking_stupid = evaluateBool(params->next->string);
			break;
			}
		}
	}

void sfSetconnectwait(strList *params)
	{
	cfg.connectwait = evaluateInt(params->string);
	}

void sfSetf6pass(strList *params)
	{
	CopyStringToBuffer(cfg.f6pass, evaluateString(params->string));
	}

void sfSetmessageroom(strList *params)
	{
	cfg.MessageRoom = evaluateInt(params->string);
	}

void sfSetchaton(strList *params)
	{
	cfg.noChat = !evaluateBool(params->string);
	}

void sfSetbells(strList *params)
	{
	cfg.noBells = evaluateInt(params->string);
	}

void sfSetmcion(strList *params)
	{
	cfg.mci = evaluateBool(params->string);
	}

void sfSetmciname(strList *params)
	{
	CopyStringToBuffer(cfg.mci_name, evaluateString(params->string));
	}

void sfSetmcifirstname(strList *params)
	{
	CopyStringToBuffer(cfg.mci_firstname, evaluateString(params->string));
	}

void sfSetmcitime(strList *params)
	{
	CopyStringToBuffer(cfg.mci_time, evaluateString(params->string));
	}

void sfSetmcidate(strList *params)
	{
	CopyStringToBuffer(cfg.mci_date, evaluateString(params->string));
	}

void sfSetmcipoop(strList *params)
	{
	CopyStringToBuffer(cfg.mci_poop, evaluateString(params->string));
	}

void sfSetcheckcts(strList *params)
	{
	cfg.checkCTS = evaluateBool(params->string);
	}

void sfSetpoop(strList *params)
	{
	cfg.poop = evaluateInt(params->string);
	}

void sfSetprinterprompt(strList *params)
	{
	cfg.printerprompt = evaluateBool(params->string);
	}

void sfSetscrolltimeout(strList *params)
	{
	cfg.scrollTimeOut = evaluateInt(params->string);
	}

void sfSetmsgcompress(strList *params)
	{
	cfg.msgCompress = evaluateBool(params->string);
	}

void sfSetrestoremode(strList *params)
	{
	cfg.restore_mode = evaluateBool(params->string);
	}

void sfSetexpire(strList *params)
	{
	cfg.expire = evaluateInt(params->string);
	}

void sfSetfuelbarempty(strList *params)
	{
	cfg.fuelbarempty = (char) evaluateInt(params->string);
	}

void sfSetfuelbarfull(strList *params)
	{
	cfg.fuelbarfull = (char) evaluateInt(params->string);
	}

void sfGetSoftVerb(strList *)
	{
	SETSTR(cfg.softverb);
	}

void sfSetSoftVerb(strList *params)
	{
	CopyStringToBuffer(cfg.softverb, evaluateString(params->string));
	}

void sfGetVersion(strList *params)
	{
	switch (evaluateInt(params->string))
		{
		case 0:
			{
#ifdef AUXMEM
			SETINT(1);
#else
#ifdef WINCIT
			SETINT(2);
#else
			SETINT(0);
#endif
#endif
			break;
			}

		case 1:
			{
			SETLONG(NumericVer);
			break;
			}

		case 2:
			{
			SETSTR(version);
			break;
			}

		default:
			{
			SETINT(-1);
			break;
			}
		}
	}

void sfGetUppercaseMsgNym(strList *)
	{
	SETSTR(cfg.Umsg_nym);
	}

void sfGetUppercaseMsgsNym(strList *)
	{
	SETSTR(cfg.Umsgs_nym);
	}

void sfGetUppercaseRoomNym(strList *)
	{
	SETSTR(cfg.Uroom_nym);
	}

void sfGetUppercaseRoomsNym(strList *)
	{
	SETSTR(cfg.Urooms_nym);
	}

void sfGetUppercaseHallNym(strList *)
	{
	SETSTR(cfg.Uhall_nym);
	}

void sfGetUppercaseHallsNym(strList *)
	{
	SETSTR(cfg.Uhalls_nym);
	}

void sfGetUppercaseUserNym(strList *)
	{
	SETSTR(cfg.Uuser_nym);
	}

void sfGetUppercaseUsersNym(strList *)
	{
	SETSTR(cfg.Uusers_nym);
	}

void sfGetUppercaseGroupNym(strList *)
	{
	SETSTR(cfg.Ugroup_nym);
	}

void sfGetUppercaseGroupsNym(strList *)
	{
	SETSTR(cfg.Ugroups_nym);
	}

void sfGetUppercaseCreditNym(strList *)
	{
	SETSTR(cfg.Ucredit_nym);
	}

void sfGetUppercaseCreditsNym(strList *)
	{
	SETSTR(cfg.Ucredits_nym);
	}

void sfGetLowercaseMsgNym(strList *)
	{
	SETSTR(cfg.Lmsg_nym);
	}

void sfGetLowercaseMsgsNym(strList *)
	{
	SETSTR(cfg.Lmsgs_nym);
	}

void sfGetLowercaseRoomNym(strList *)
	{
	SETSTR(cfg.Lroom_nym);
	}

void sfGetLowercaseRoomsNym(strList *)
	{
	SETSTR(cfg.Lrooms_nym);
	}

void sfGetLowercaseHallNym(strList *)
	{
	SETSTR(cfg.Lhall_nym);
	}

void sfGetLowercaseHallsNym(strList *)
	{
	SETSTR(cfg.Lhalls_nym);
	}

void sfGetLowercaseUserNym(strList *)
	{
	SETSTR(cfg.Luser_nym);
	}

void sfGetLowercaseUsersNym(strList *)
	{
	SETSTR(cfg.Lusers_nym);
	}

void sfGetLowercaseGroupNym(strList *)
	{
	SETSTR(cfg.Lgroup_nym);
	}

void sfGetLowercaseGroupsNym(strList *)
	{
	SETSTR(cfg.Lgroups_nym);
	}

void sfGetLowercaseCreditNym(strList *)
	{
	SETSTR(cfg.Lcredit_nym);
	}

void sfGetLowercaseCreditsNym(strList *)
	{
	SETSTR(cfg.Lcredits_nym);
	}

