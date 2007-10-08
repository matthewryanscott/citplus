// the script function array poopie.

typedef struct ifr
	{
	void (*func)(strList *);
	} internalFunctionRecord;

void sfPrintm(strList *);
void sfOutstrpaced(strList *);
void sfPrintc(strList *);
void sfGetstr(strList *);
void sfWaitfor(strList *);
void sfRandom(strList *);
void sfSetRandom(strList *);
void sfDoCR(strList *);
void sfDoccr(strList *);
void sfDiskfree(strList *);
void sfThisroom(strList *);
void sfGetroomdir(strList *);
void sfGetmaxhalls(strList *);
void sfGetmaxgroups(strList *);
void sfGetmaxrooms(strList *);
void sfGetnmessages(strList *);
void sfGetmaxlogtab(strList *);
void sfIsgrpinuse(strList *);
void sfIsgrplocked(strList *);
void sfIsgrphidden(strList *);
void sfIsgrpautoadd(strList *);
void sfGetgrpname(strList *);
void sfGetgrpdesc(strList *);
void sfIsingroup(strList *);
void sfGetcallno(strList *);
void sfGetoldest(strList *);
void sfGetnewest(strList *);
void sfGetmtoldest(strList *);
void sfGetnodename(strList *);
void sfGetsysopname(strList *);
void sfGetforwardsys(strList *);
void sfGetnoderegion(strList *);
void sfGetnodecountry(strList *);
void sfGetCit86Country(strList *);
void sfGetCit86Domain(strList *);
void sfGetnodesig(strList *);
void sfGettwitregion(strList *);
void sfGettwitcountry(strList *);
void sfGetalias(strList *);
void sfGetLocID(strList *);
void sfGetnodephone(strList *);
void sfGetmdata(strList *);
void sfGetdumbmodem(strList *);
void sfGetinitbaud(strList *);
void sfGetminbaud(strList *);
void sfGetmodsetup(strList *);
void sfGetmodunsetup(strList *);
void sfGetdownshift(strList *);
void sfGetdialsetup(strList *);
void sfGetdialpref(strList *);
void sfGetdialring(strList *);
void sfGetupday(strList *);
void sfGetuphour(strList *);
void sfGethangupdelay(strList *);
void sfGethangup(strList *);
void sfGetoffhookstr(strList *);
void sfGethomepath(strList *);
void sfGetmsgpath(strList *);
void sfGethelppath(strList *);
void sfGethelppath2(strList *);
void sfGettemppath(strList *);
void sfGetroompath(strList *);
void sfGettrapfile(strList *);
void sfGetapplpath(strList *);
void sfGetdirpath(strList *);
void sfGettranspath(strList *);
void sfGetprinter(strList *);
void sfGetdlpath(strList *);
void sfGetlogextdir(strList *);
void sfGetautoansi(strList *);
void sfGetentername(strList *);
void sfGetnewuserapp(strList *);
void sfGetoldcount(strList *);
void sfGetlclosedsys(strList *);
void sfGetlverified(strList *);
void sfGetlquestionnaire(strList *);
void sfGetlsysmsg(strList *);
void sfGetlcreate(strList *);
void sfGetunlogtimeout(strList *);
void sfGettimeout(strList *);
void sfGetunlogbal(strList *);
void sfGetdefdatestamp(strList *);
void sfGetdefvdatestamp(strList *);
void sfGetdefrprompt(strList *);
void sfGetmsgverb(strList *);
void sfGetborder(strList *);
void sfGetdefnetprefix(strList *);
void sfGetnopwecho(strList *);
void sfGetanonauthor(strList *);
void sfGettwirly(strList *);
void sfGettwirlypause(strList *);
void sfGetmessagek(strList *);
void sfGetmaxfiles(strList *);
void sfGetsetmsgnym(strList *);
void sfGetsysborders(strList *);
void sfGetforcelogin(strList *);
void sfGetoffhook(strList *);
void sfGetaccounting(strList *);
void sfGetnetmail(strList *);
void sfGetsubhubs(strList *);
void sfGetcolors(strList *);
void sfGetmoreprompt(strList *);
void sfGetsleepprompt(strList *);
void sfGetsetcensor(strList *);
void sfGetcountbeep(strList *);
void sfGetsleepcount(strList *);
void sfGetdialmacro(strList *);
void sfGetdiskfree(strList *);
void sfGetaidechathour(strList *);
void sfGetentersur(strList *);
void sfGetaidehall(strList *);
void sfGetreadluser(strList *);
void sfGetenterok(strList *);
void sfGetreadok(strList *);
void sfGetroomok(strList *);
void sfGetaidemoderate(strList *);
void sfGetsurnames(strList *);
void sfGetnetsurnames(strList *);
void sfGettitles(strList *);
void sfGetnettitles(strList *);
void sfGetbios(strList *);
void sfGetattr(strList *);
void sfGetwattr(strList *);
void sfGetcattr(strList *);
void sfGetbattr(strList *);
void sfGetuttr(strList *);
void sfGetscreensave(strList *);
void sfGetconnectwait(strList *);
void sfGetidletimeout(strList *);
void sfGetf6pass(strList *);
void sfGetmessageroom(strList *);
void sfGetchaton(strList *);
void sfGetbells(strList *);
void sfGetmcion(strList *);
void sfGetmciname(strList *);
void sfGetmcifirstname(strList *);
void sfGetmcitime(strList *);
void sfGetmcidate(strList *);
void sfGetmcipoop(strList *);
void sfGetvmemfile(strList *);
void sfGetcheckcts(strList *);
void sfGetpoop(strList *);
void sfGetmaxpoop(strList *);
void sfGetpoopuser(strList *);
void sfGetprinterprompt(strList *);
void sfGetscrollsize(strList *);
void sfGetscrollcolors(strList *);
void sfGetscrolltimeout(strList *);
void sfGetmsgcompress(strList *);
void sfGetrestoremode(strList *);
void sfGetexpire(strList *);
void sfGetmaxjumpback(strList *);
void sfGetfuelbarempty(strList *);
void sfGetfuelbarfull(strList *);
void sfGetstatnum(strList *);
void sfGetgroupslot(strList *);
void sfAdduser(strList *);
void sfRemoveuser(strList *);
void sfAskyn(strList *);
void sfIsroominuse(strList *);
void sfIsroompublic(strList *);
void sfIsroomdir(strList *);
void sfIsroomperm(strList *);
void sfIsroomgrponly(strList *);
void sfIsroomreadonly(strList *);
void sfIsroomdownonly(strList *);
void sfIsroomshared(strList *);
void sfIsroommoderated(strList *);
void sfIsroomapplic(strList *);
void sfIsroombio(strList *);
void sfIsroomuponly(strList *);
void sfIsroomprivgrp(strList *);
void sfIsroomanon(strList *);
void sfIsroomsubject(strList *);
void sfIsroomgrpmod(strList *);
void sfGetroomname(strList *);
void sfGetnetid(strList *);
void sfGetroomgroup(strList *);
void sfGetroomtotal(strList *);
void sfGetroommsgs(strList *);
void sfGetroomnew(strList *);
void sfGetroomentered(strList *);
void sfGetroomhasmail(strList *);
void sfGetroomvisited(strList *);
void sfGetroombypass(strList *);
void sfGethallname(strList *);
void sfGethallgroup(strList *);
void sfIshallinuse(strList *);
void sfIshallgrp(strList *);
void sfIshalldesc(strList *);
void sfIshallenterroom(strList *);
void sfGethalldesc(strList *);
void sfGetusername(strList *);
void sfGetuserin(strList *);
void sfGetuserpw(strList *);
void sfGetuserfwd(strList *);
void sfGetuserfwdnode(strList *);
void sfGetusersurname(strList *);
void sfGetusertitle(strList *);
void sfGetuserdefhall(strList *);
void sfGetusernulls(strList *);
void sfGetuserwidth(strList *);
void sfGetusercalltime(strList *);
void sfGetusercallno(strList *);
void sfGetusercredits(strList *);
void sfGetuserlastpointer(strList *);
void sfGetuserlasthall(strList *);
void sfGetuserlastroom(strList *);
void sfGetuserlines(strList *);
void sfGetuserproto(strList *);
void sfGetuserprompt(strList *);
void sfGetuserdstamp(strList *);
void sfGetuservdstamp(strList *);
void sfGetusersig(strList *);
void sfGetusernetpref(strList *);
void sfGetuseraddr1(strList *);
void sfGetuseraddr2(strList *);
void sfGetuseraddr3(strList *);
void sfGetuserpoop(strList *);
void sfIsuserinuse(strList *);
void sfIsuseruponly(strList *);
void sfIsuserlf(strList *);
void sfIsuserexpert(strList *);
void sfIsuseraide(strList *);
void sfIsusertabs(strList *);
void sfIsuserlastold(strList *);
void sfIsuserproblem(strList *);
void sfIsuserunlisted(strList *);
void sfIsuserperm(strList *);
void sfIsusersysop(strList *);
void sfIsusernode(strList *);
void sfIsusernet(strList *);
void sfIsusernoacct(strList *);
void sfIsusernomail(strList *);
void sfIsuserroomtell(strList *);
void sfIsuserfwdnode(strList *);
void sfIsusernexthall(strList *);
void sfIsuserentborders(strList *);
void sfIsuserverified(strList *);
void sfIsusersurlock(strList *);
void sfIsuserhalllock(strList *);
void sfIsuserpsycho(strList *);
void sfIsuserdisplayts(strList *);
void sfIsusersubjects(strList *);
void sfIsusersignatures(strList *);
void sfIsusertwirly(strList *);
void sfIsuserverbose(strList *);
void sfIsusermsgpause(strList *);
void sfIsuserminibin(strList *);
void sfIsusermsgcls(strList *);
void sfIsuserroominfo(strList *);
void sfIsuserhalltell(strList *);
void sfIsuservcont(strList *);
void sfIsuserviewcensor(strList *);
void sfIsuserseeborders(strList *);
void sfIsuserout300(strList *);
void sfIsuserlocksig(strList *);
void sfIsuserhideexcl(strList *);
void sfIsusernodownload(strList *);
void sfIsusernoupload(strList *);
void sfIsusernochat(strList *);
void sfIsuserprint(strList *);
void sfGetuserrealname(strList *);
void sfGetuserphone(strList *);
void sfSetusercredits(strList *);
void sfRunapplic(strList *);
void sfChatreq(strList *);
void sfPlaysound(strList *);
void sfSayascii(strList *);
void sfGetconnectrate(strList *);
void sfGetportrate(strList *);
void sfSetportrate(strList *);
void sfChat(strList *);
void sfDochat(strList *);
void sfGetmaxborders(strList *);
void sfSetidletimeout(strList *);
void sfOpenfile(strList *);
void sfClosefile(strList *);
void sfDeletefile(strList *);
void sfReadstr(strList *);
void sfReadint(strList *);
void sfReaduint(strList *);
void sfReadlong(strList *);
void sfReadulong(strList *);
void sfReadbool(strList *);
void sfWritestr(strList *);
void sfWriteint(strList *);
void sfWriteuint(strList *);
void sfWritelong(strList *);
void sfWriteulong(strList *);
void sfWritebool(strList *);
void sfSeekfile(strList *);
void sfTellfile(strList *);
void sfGetltpwhash(strList *);
void sfGetltinhash(strList *);
void sfGetltname(strList *);
void sfGetltalias(strList *);
void sfGetltlocid(strList *);
void sfGetltlogslot(strList *);
void sfGetltcallno(strList *);
void sfGetltinuse(strList *);
void sfGetltucmask(strList *);
void sfGetltlfmask(strList *);
void sfGetltexpert(strList *);
void sfGetltaide(strList *);
void sfGetlttabs(strList *);
void sfGetltoldtoo(strList *);
void sfGetltproblem(strList *);
void sfGetltunlisted(strList *);
void sfGetltpermanent(strList *);
void sfGetltsysop(strList *);
void sfGetltnode(strList *);
void sfGetltnetuser(strList *);
void sfGetltnoaccount(strList *);
void sfGetltnomail(strList *);
void sfGetltroomtell(strList *);
void sfGetparam(strList *);
void sfGetparamcount(strList *);
void sfGetroomhasoldmail(strList *);
void sfIchar(strList *);
void sfIcharne(strList *);
void sfDumpf(strList *);
void sfDumpfc(strList *);
void sfPause(strList *);
void sfWriteapl(strList *);
void sfFileexists(strList *);
void sfFileeof(strList *);
void sfMsgAppendText(strList *);
void sfMsgClear(strList *);
void sfMsgDump(strList *);
void sfMsgGetAuthor(strList *);
void sfMsgGetCit86Country(strList *);
void sfMsgGetCopyOfMessage(strList *);
void sfMsgGetCreationRoom(strList *);
void sfMsgGetCreationTime(strList *);
void sfMsgGetDestinationAddress(strList *);
void sfMsgGetEncryptionKey(strList *);
void sfMsgGetEZCreationTime(strList *);
void sfMsgGetFileLink(strList *);
void sfMsgGetForward(strList *);
void sfMsgGetFromPath(strList *);
void sfMsgGetGroup(strList *);
void sfMsgGetHeadLoc(strList *);
void sfMsgGetLocalID(strList *);
void sfMsgGetOriginalAttribute(strList *);
void sfMsgGetOriginalID(strList *);
void sfMsgGetOriginalRoom(strList *);
void sfMsgGetOriginCountry(strList *);
void sfMsgGetOriginNodeName(strList *);
void sfMsgGetOriginPhoneNumber(strList *);
void sfMsgGetOriginRegion(strList *);
void sfMsgGetOriginSoftware(strList *);
void sfMsgGetRealName(strList *);
void sfMsgGetReplyToMessage(strList *);
void sfMsgGetRoomNumber(strList *);
void sfMsgGetSignature(strList *);
void sfMsgGetSourceID(strList *);
void sfMsgGetSourceRoomName(strList *);
void sfMsgGetSubject(strList *);
void sfMsgGetSurname(strList *);
void sfMsgGetTitle(strList *);
void sfMsgGetToCountry(strList *);
void sfMsgGetToNodeName(strList *);
void sfMsgGetToPath(strList *);
void sfMsgGetToPhoneNumber(strList *);
void sfMsgGetToRegion(strList *);
void sfMsgGetToUser(strList *);
void sfMsgGetTwitCountry(strList *);
void sfMsgGetTwitRegion(strList *);
void sfMsgGetUserSignature(strList *);
void sfMsgGetX(strList *);
void sfMsgHeaderClear(strList *);
void sfMsgIsCensored(strList *);
void sfMsgIsCompressed(strList *);
void sfMsgIsEncrypted(strList *);
void sfMsgIsLocal(strList *);
void sfMsgIsMadeVisible(strList *);
void sfMsgIsReceiptConfirmationRequested(strList *);
void sfMsgIsReceived(strList *);
void sfMsgIsRepliedTo(strList *);
void sfMsgIsViewDuplicate(strList *);
void sfMsgLoad(strList *);
void sfMsgSave(strList *);
void sfMsgSetAuthor(strList *);
void sfMsgSetCensored(strList *);
void sfMsgSetCreationRoom(strList *);
void sfMsgSetCreationTime(strList *);
void sfMsgSetDestinationAddress(strList *);
void sfMsgSetEncrypted(strList *);
void sfMsgSetEncryptionKey(strList *);
void sfMsgSetEZCreationTime(strList *);
void sfMsgSetFileLink(strList *);
void sfMsgSetForward(strList *);
void sfMsgSetGroup(strList *);
void sfMsgSetLocal(strList *);
void sfMsgSetOriginalRoom(strList *);
void sfMsgSetRealName(strList *);
void sfMsgSetReceiptConfirmationRequested(strList *);
void sfMsgSetReplyToMessage(strList *);
void sfMsgSetRoomNumber(strList *);
void sfMsgSetSignature(strList *);
void sfMsgSetSourceRoomName(strList *);
void sfMsgSetSubject(strList *);
void sfMsgSetSurname(strList *);
void sfMsgSetText(strList *);
void sfMsgSetTitle(strList *);
void sfMsgSetToPath(strList *);
void sfMsgSetToPhoneNumber(strList *);
void sfMsgSetToUser(strList *);
void sfMsgSetUserSignature(strList *);
void sfMsgSetX(strList *);
void sfMsgShow(strList *);
void sfMsgStart(strList *);
void sfGetFormattedString(strList *);
void sfFindRoomByName(strList *);
void sfSetDebugScript(strList *);
void sfPauseScript(strList *);
void sfDebuggingMsg(strList *);
void sfSetBreakpoint(strList *);
void sfClearBreakpoint(strList *);
void sfAddWatch(strList *);
void sfRemoveWatch(strList *);
void sfAsciitochar(strList *);
void sfChartoascii(strList *);
void sfSformat(strList *);
void sfStrftime(strList *);
void sfGetcurrenttime(strList *);
void sfNet69fetch(strList *);
void sfNet69incorporate(strList *);
void sfNet86fetch(strList *);
void sfNet86incorporate(strList *);
void sfChangedir(strList *);
void sfHangup(strList *);
void sfDial(strList *);
void sfModemflush(strList *);
void sfModeminitport(strList *);
void sfUserAddKillNode(strList *);
void sfUserAddKillRegion(strList *);
void sfUserAddKillText(strList *);
void sfUserAddKillUser(strList *);
void sfUserAddWordToDictionary(strList *);
void sfUserClear(strList *);
void sfUserDump(strList *);
void sfUserGetAlias(strList *);
void sfUserGetBirthDate(strList *);
void sfUserGetCallLimit(strList *);
void sfUserGetCallNumber(strList *);
void sfUserGetCallsToday(strList *);
void sfUserGetCallTime(strList *);
void sfUserGetCredits(strList *);
void sfUserGetDateStamp(strList *);
void sfUserGetDefaultHall(strList *);
void sfUserGetDefaultProtocol(strList *);
void sfUserGetDefaultRoom(strList *);
void sfUserGetDL_Bytes(strList *);
void sfUserGetDL_Num(strList *);
void sfUserGetFirstOn(strList *);
void sfUserGetForwardAddr(strList *);
void sfUserGetForwardAddrNode(strList *);
void sfUserGetForwardAddrRegion(strList *);
void sfUserGetInitials(strList *);
void sfUserGetKillNode(strList *);
void sfUserGetKillRegion(strList *);
void sfUserGetKillText(strList *);
void sfUserGetKillUser(strList *);
void sfUserGetLastHall(strList *);
void sfUserGetLastMessage(strList *);
void sfUserGetLastRoom(strList *);
void sfUserGetLinesPerScreen(strList *);
void sfUserGetLocID(strList *);
void sfUserGetLogins(strList *);
void sfUserGetMailAddr1(strList *);
void sfUserGetMailAddr2(strList *);
void sfUserGetMailAddr3(strList *);
void sfUserGetMorePrompt(strList *);
void sfUserGetName(strList *);
void sfUserGetNetPrefix(strList *);
void sfUserGetNulls(strList *);
void sfUserGetNumKillNode(strList *);
void sfUserGetNumKillRegion(strList *);
void sfUserGetNumKillText(strList *);
void sfUserGetNumKillUser(strList *);
void sfUserGetNumUserShow(strList *);
void sfUserGetOccupation(strList *);
void sfUserGetPassword(strList *);
void sfUserGetPasswordChangeTime(strList *);
void sfUserGetPhoneNumber(strList *);
void sfUserGetPoopcount(strList *);
void sfUserGetPosted(strList *);
void sfUserGetPromptFormat(strList *);
void sfUserGetRead(strList *);
void sfUserGetRealName(strList *);
void sfUserGetRoomNewPointer(strList *);
void sfUserGetSignature(strList *);
void sfUserGetSpellCheckMode(strList *);
void sfUserGetSurname(strList *);
void sfUserGetTitle(strList *);
void sfUserGetTotalTime(strList *);
void sfUserGetUL_Bytes(strList *);
void sfUserGetUL_Num(strList *);
void sfUserGetVerboseDateStamp(strList *);
void sfUserGetWhereHear(strList *);
void sfUserGetWidth(strList *);
void sfUserIsAccounting(strList *);
void sfUserIsAide(strList *);
void sfUserIsAutoNextHall(strList *);
void sfUserIsAutoVerbose(strList *);
void sfUserIsChat(strList *);
void sfUserIsCheckAllCaps(strList *);
void sfUserIsCheckApostropheS(strList *);
void sfUserIsCheckDigits(strList *);
void sfUserIsClearScreenBetweenMessages(strList *);
void sfUserIsConfirmNoEO(strList *);
void sfUserIsDefaultHallLocked(strList *);
void sfUserIsDownload(strList *);
void sfUserIsDungeoned(strList *);
void sfUserIsEnterBorders(strList *);
void sfUserIsExcludeEncryptedMessages(strList *);
void sfUserIsExpert(strList *);
void sfUserIsForwardToNode(strList *);
void sfUserIsHideMessageExclusions(strList *);
void sfUserIsIBMRoom(strList *);
void sfUserIsInGroup(strList *);
void sfUserIsInRoom(strList *);
void sfUserIsInuse(strList *);
void sfUserIsKillNode(strList *);
void sfUserIsKillRegion(strList *);
void sfUserIsKillText(strList *);
void sfUserIsKillUser(strList *);
void sfUserIsLinefeeds(strList *);
void sfUserIsMail(strList *);
void sfUserIsMainSysop(strList *);
void sfUserIsMakeRoom(strList *);
void sfUserGetSex(strList *);
void sfUserIsMinibin(strList *);
void sfUserIsMusic(strList *);
void sfUserIsNetUser(strList *);
void sfUserIsNode(strList *);
void sfUserIsOldToo(strList *);
void sfUserIsOut300(strList *);
void sfUserIsPauseBetweenMessages(strList *);
void sfUserIsPermanent(strList *);
void sfUserIsPrintFile(strList *);
void sfUserIsProblem(strList *);
void sfUserIsPsycho(strList *);
void sfUserIsPUnPauses(strList *);
void sfUserIsRoomExcluded(strList *);
void sfUserIsRoomInPersonalHall(strList *);
void sfUserIsSurnameLocked(strList *);
void sfUserIsSysop(strList *);
void sfUserIsTabs(strList *);
void sfUserIsTwirly(strList *);
void sfUserIsUnlisted(strList *);
void sfUserIsUpload(strList *);
void sfUserIsUpperOnly(strList *);
void sfUserIsUsePersonalHall(strList *);
void sfUserIsUserSignatureLocked(strList *);
void sfUserIsVerboseLogOut(strList *);
void sfUserIsVerified(strList *);
void sfUserIsViewBorders(strList *);
void sfUserIsViewCensoredMessages(strList *);
void sfUserIsViewCommas(strList *);
void sfUserIsViewHallDescription(strList *);
void sfUserIsViewRoomDesc(strList *);
void sfUserIsViewRoomInfoLines(strList *);
void sfUserIsViewSignatures(strList *);
void sfUserIsViewSubjects(strList *);
void sfUserIsViewTitleSurname(strList *);
void sfUserIsWideRoom(strList *);
void sfUserIsWordInDictionary(strList *);
void sfUserIsYouAreHere(strList *);
void sfUserLoad(strList *);
void sfUserRemoveKillNode(strList *);
void sfUserRemoveKillRegion(strList *);
void sfUserRemoveKillText(strList *);
void sfUserRemoveKillUser(strList *);
void sfUserRemoveWordFromDictionary(strList *);
void sfUserSave(strList *);
void sfUserSetAccounting(strList *);
void sfUserSetAide(strList *);
void sfUserSetAlias(strList *);
void sfUserSetAutoNextHall(strList *);
void sfUserSetAutoVerbose(strList *);
void sfUserSetBirthDate(strList *);
void sfUserSetCallLimit(strList *);
void sfUserSetCallNumber(strList *);
void sfUserSetCallsToday(strList *);
void sfUserSetCallTime(strList *);
void sfUserSetChat(strList *);
void sfUserSetCheckAllCaps(strList *);
void sfUserSetCheckApostropheS(strList *);
void sfUserSetCheckDigits(strList *);
void sfUserSetClearScreenBetweenMessages(strList *);
void sfUserSetConfirmNoEO(strList *);
void sfUserSetCredits(strList *);
void sfUserSetDateStamp(strList *);
void sfUserSetDefaultHall(strList *);
void sfUserSetDefaultHallLocked(strList *);
void sfUserSetDefaultProtocol(strList *);
void sfUserSetDefaultRoom(strList *);
void sfUserSetDL_Bytes(strList *);
void sfUserSetDL_Num(strList *);
void sfUserSetDownload(strList *);
void sfUserSetDungeoned(strList *);
void sfUserSetEnterBorders(strList *);
void sfUserSetExcludeEncryptedMessages(strList *);
void sfUserSetExpert(strList *);
void sfUserSetFirstOn(strList *);
void sfUserSetForwardAddr(strList *);
void sfUserSetForwardAddrNode(strList *);
void sfUserSetForwardAddrRegion(strList *);
void sfUserSetForwardToNode(strList *);
void sfUserSetHideMessageExclusions(strList *);
void sfUserSetIBMRoom(strList *);
void sfUserSetInGroup(strList *);
void sfUserSetInitials(strList *);
void sfUserSetInRoom(strList *);
void sfUserSetInuse(strList *);
void sfUserSetLastHall(strList *);
void sfUserSetLastMessage(strList *);
void sfUserSetLastRoom(strList *);
void sfUserSetLinefeeds(strList *);
void sfUserSetLinesPerScreen(strList *);
void sfUserSetLocID(strList *);
void sfUserSetLogins(strList *);
void sfUserSetMail(strList *);
void sfUserSetMailAddr1(strList *);
void sfUserSetMailAddr2(strList *);
void sfUserSetMailAddr3(strList *);
void sfUserSetMakeRoom(strList *);
void sfUserSetSex(strList *);
void sfUserSetMinibin(strList *);
void sfUserSetMorePrompt(strList *);
void sfUserSetMusic(strList *);
void sfUserSetName(strList *);
void sfUserSetNetPrefix(strList *);
void sfUserSetNetUser(strList *);
void sfUserSetNode(strList *);
void sfUserSetNulls(strList *);
void sfUserSetNumUserShow(strList *);
void sfUserSetOccupation(strList *);
void sfUserSetOldToo(strList *);
void sfUserSetOut300(strList *);
void sfUserSetPassword(strList *);
void sfUserSetPasswordChangeTime(strList *);
void sfUserSetPauseBetweenMessages(strList *);
void sfUserSetPermanent(strList *);
void sfUserSetPhoneNumber(strList *);
void sfUserSetPoopcount(strList *);
void sfUserSetPosted(strList *);
void sfUserSetPrintFile(strList *);
void sfUserSetProblem(strList *);
void sfUserSetPromptFormat(strList *);
void sfUserSetPsycho(strList *);
void sfUserSetPUnPauses(strList *);
void sfUserSetRead(strList *);
void sfUserSetRealName(strList *);
void sfUserSetRoomExcluded(strList *);
void sfUserSetRoomInPersonalHall(strList *);
void sfUserSetRoomNewPointer(strList *);
void sfUserSetSignature(strList *);
void sfUserSetSpellCheckMode(strList *);
void sfUserSetSurname(strList *);
void sfUserSetSurnameLocked(strList *);
void sfUserSetSysop(strList *);
void sfUserSetTabs(strList *);
void sfUserSetTitle(strList *);
void sfUserSetTotalTime(strList *);
void sfUserSetTwirly(strList *);
void sfUserSetUL_Bytes(strList *);
void sfUserSetUL_Num(strList *);
void sfUserSetUnlisted(strList *);
void sfUserSetUpload(strList *);
void sfUserSetUpperOnly(strList *);
void sfUserSetUsePersonalHall(strList *);
void sfUserSetUserSignatureLocked(strList *);
void sfUserSetVerboseDateStamp(strList *);
void sfUserSetVerboseLogOut(strList *);
void sfUserSetVerified(strList *);
void sfUserSetViewBorders(strList *);
void sfUserSetViewCensoredMessages(strList *);
void sfUserSetViewCommas(strList *);
void sfUserSetViewHallDescription(strList *);
void sfUserSetViewRoomDesc(strList *);
void sfUserSetViewRoomInfoLines(strList *);
void sfUserSetViewSignatures(strList *);
void sfUserSetViewSubjects(strList *);
void sfUserSetViewTitleSurname(strList *);
void sfUserSetWhereHear(strList *);
void sfUserSetWideRoom(strList *);
void sfUserSetWidth(strList *);
void sfUserSetYouAreHere(strList *);
void sfUserStart(strList *);
void sfMemoryFree(strList *);
void sfUserGetUserDefined(strList *);
void sfUserSetUserDefined(strList *);
void sfUserIsUserDefined(strList *);
void sfUserRemoveUserDefined(strList *);
void sfKillRoom(strList *);
void sfKillUser(strList *);
void sfSetnodename(strList *);
void sfSetsysopname(strList *);
void sfSetforwardsys(strList *);
void sfSetnoderegion(strList *);
void sfSetnodecountry(strList *);
void sfSetCit86Country(strList *);
void sfSetCit86Domain(strList *);
void sfSettwitregion(strList *);
void sfSettwitcountry(strList *);
void sfSetnodephone(strList *);
void sfSetmdata(strList *);
void sfSetdumbmodem(strList *);
void sfSetinitbaud(strList *);
void sfSetminbaud(strList *);
void sfSetmodsetup(strList *);
void sfSetmodunsetup(strList *);
void sfSetdownshift(strList *);
void sfSetdialsetup(strList *);
void sfSetdialpref(strList *);
void sfSetdialring(strList *);
void sfSetupday(strList *);
void sfSetuphour(strList *);
void sfSethangupdelay(strList *);
void sfSethangup(strList *);
void sfSetoffhookstr(strList *);
void sfSethomepath(strList *);
void sfSetmsgpath(strList *);
void sfSethelppath(strList *);
void sfSethelppath2(strList *);
void sfSettemppath(strList *);
void sfSetroompath(strList *);
void sfSettrapfile(strList *);
void sfSetapplpath(strList *);
void sfSetdirpath(strList *);
void sfSettranspath(strList *);
void sfSetprinter(strList *);
void sfSetdlpath(strList *);
void sfSetlogextdir(strList *);
void sfSetautoansi(strList *);
void sfSetentername(strList *);
void sfSetnewuserapp(strList *);
void sfSetoldcount(strList *);
void sfSetlclosedsys(strList *);
void sfSetlverified(strList *);
void sfSetlsysmsg(strList *);
void sfSetlcreate(strList *);
void sfSetunlogtimeout(strList *);
void sfSettimeout(strList *);
void sfSetunlogbal(strList *);
void sfSetdefdatestamp(strList *);
void sfSetdefvdatestamp(strList *);
void sfSetdefrprompt(strList *);
void sfSetmsgnym(strList *);
void sfSetmsgsnym(strList *);
void sfSetcreditnym(strList *);
void sfSetcreditsnym(strList *);
void sfSetmsgverb(strList *);
void sfSetborder(strList *);
void sfSetdefnetprefix(strList *);
void sfSetnopwecho(strList *);
void sfSetanonauthor(strList *);
void sfSettwirly(strList *);
void sfSettwirlypause(strList *);
void sfSetmaxfiles(strList *);
void sfSetsetmsgnym(strList *);
void sfSetforcelogin(strList *);
void sfSetoffhook(strList *);
void sfSetnetmail(strList *);
void sfSetsubhubs(strList *);
void sfSetcolors(strList *);
void sfSetmoreprompt(strList *);
void sfSetsleepprompt(strList *);
void sfSetsetcensor(strList *);
void sfSetcountbeep(strList *);
void sfSetsleepcount(strList *);
void sfSetdialmacro(strList *);
void sfSetdiskfree(strList *);
void sfSetaidechathour(strList *);
void sfSetentersur(strList *);
void sfSetaidehall(strList *);
void sfSetreadluser(strList *);
void sfSetenterok(strList *);
void sfSetreadok(strList *);
void sfSetroomok(strList *);
void sfSetaidemoderate(strList *);
void sfSetsurnames(strList *);
void sfSetnetsurnames(strList *);
void sfSettitles(strList *);
void sfSetnettitles(strList *);
void sfSetattr(strList *);
void sfSetwattr(strList *);
void sfSetcattr(strList *);
void sfSetbattr(strList *);
void sfSetuttr(strList *);
void sfSetscreensave(strList *);
void sfSetconnectwait(strList *);
void sfSetf6pass(strList *);
void sfSetmessageroom(strList *);
void sfSetchaton(strList *);
void sfSetbells(strList *);
void sfSetmcion(strList *);
void sfSetmciname(strList *);
void sfSetmcifirstname(strList *);
void sfSetmcitime(strList *);
void sfSetmcidate(strList *);
void sfSetmcipoop(strList *);
void sfSetcheckcts(strList *);
void sfSetpoop(strList *);
void sfSetprinterprompt(strList *);
void sfSetscrolltimeout(strList *);
void sfSetmsgcompress(strList *);
void sfSetrestoremode(strList *);
void sfSetexpire(strList *);
void sfSetfuelbarempty(strList *);
void sfSetfuelbarfull(strList *);
void sfFindUserByName(strList *);
void sfGetOneKey(strList *);
void sfIsNetIDInNetIDCit(strList *params);
void sfAddNetIDToNetIDCit(strList *params);
void sfRemoveNetIDFromNetIDCit(strList *params);
void sfGetCommentOfNetIDInNetIDCit(strList *params);
void sfFindRoomByNetID(strList *params);
void sfGetNumberOfMessages(strList *);
void sfMsgTabIsMail(strList *params);
void sfMsgTabIsReceived(strList *params);
void sfMsgTabIsRepliedTo(strList *params);
void sfMsgTabIsProblem(strList *params);
void sfMsgTabIsMadevis(strList *params);
void sfMsgTabIsLimited(strList *params);
void sfMsgTabIsModerated(strList *params);
void sfMsgTabIsCopy(strList *params);
void sfMsgTabIsNetMail(strList *params);
void sfMsgTabIsForwarded(strList *params);
void sfMsgTabIsNetworked(strList *params);
void sfMsgTabIsMassemail(strList *params);
void sfMsgTabIsCensored(strList *params);
void sfMsgTabIsLocal(strList *params);
void sfMsgTabIsInuse(strList *params);
void sfMsgTabGetLocation(strList *params);
void sfMsgTabGetRoomNum(strList *params);
void sfMsgTabGetToHash(strList *params);
void sfMsgTabGetAuthHash(strList *params);
void sfMsgTabGetOriginID(strList *params);
void sfMsgTabGetPrevRoomMsg(strList *params);
void sfMsgTabGetNextRoomMsg(strList *params);
void sfMsgTabGetCopyOffset(strList *params);
void sfMsgGetOriginAddress(strList *);
void sfStartDupCheck(strList *);
void sfAddDupCheck(strList *params);
void sfCheckForDup(strList *params);
void sfSetDebugSystem(strList *params);
void sfGetDebugSystem(strList *);
void sfSetOutImpervious(strList *);
void sfSetOutNormal(strList *);
void sfSetOutNostop(strList *);
void sfIsCanOutput(strList *);
void sfCheckUserOutControl(strList *params);
void sfIsOutputStop(strList *);
void sfIsOutputNext(strList *);
void sfIsOutputJump(strList *);
void sfKeyboardFlush(strList *);
void sfKeyboardAdd(strList *params);
void sfExitCitadel(strList *params);
void sfIsConsoleLocked(strList *params);
void sfSetConsoleLock(strList *params);
void sfGetContextSensitiveText(strList *);
void sfAppendToNetCommandReply(strList *params);
void sfGetNetCommandText(strList *);
void sfIsLoggedIn(strList *);
void sfGetDirectory(strList *params);
void sfCountDirectory(strList *);
void sfDumpDirectory(strList *);
void sfDirectoryGetTime(strList *params);
void sfDirectoryGetSize(strList *params);
void sfDirectoryGetName(strList *params);
void sfAddCommasToNumber(strList *params);
void sfStrPos(strList *params);
void sfIsFilenameLegal(strList *params);
void sfStringIsAddress(strList *params);
void sfFindNodeByNameOrAddress(strList *params);
void sfHexToDec(strList *params);
void sfDecToHex(strList *params);
void sfIsRoomInHall(strList *params);
void sfSetRoomInHall(strList *params);
void sfFindHallByName(strList *params);
void sfRenamefile(strList *params);
void sfHowLongAgoWasIt(strList *params);
void sfDoNetwork(strList *params);
void sfMsgSaveTextToFile(strList *params);
void sfGetNetCommandTextToFile(strList *);
void sfMsgIsMoreFlag(strList *params);
void sfMsgSetMoreFlag(strList *params);
void sfMsgGetCommentCount(strList *);
void sfMsgGetComment(strList *params);
void sfMsgAddComment(strList *params);
void sfNet69RoomReq(strList *params);
void sfUserUseCurrentUser(strList *);
void sfParseDateString(strList *params);
void sfParseTimeString(strList *params);
void sfIsMatchWildcard(strList *params);
void sfReadAplOnExit(strList *);
void sfGetModemStr(strList *params);
void sfIsroomwindow(strList *params);
void sfSetRoomBypassed(strList *params);
void sfIsRoomBypassed(strList *params);
void sfReadChar(strList *params);
void sfWriteChar(strList *params);
void sfStartMessageEditor(strList *params);
void sfMsgSetPublicToUser(strList *);
void sfMsgGetPublicToUser(strList *);
void sfMsgSetFromPath(strList *params);
void sfUserIsRoman(strList *);
void sfUserSetRoman(strList *params);
void sfUserIsSuperSysop(strList *);
void sfUserSetSuperSysop(strList *);
void sfMsgLoadTextFromFile(strList *params);
void sfGetVersion(strList *params);
void sfHasTermWindow(strList *);
void sfRoomChangeDir(strList *);
void sfRoomClear(strList *);
void sfRoomDump(strList *);
void sfRoomGetApplicationName(strList *);
void sfRoomGetArchiveFile(strList *);
void sfRoomGetDescFile(strList *);
void sfRoomGetDictionary(strList *);
void sfRoomGetDirectory(strList *);
void sfRoomGetGroup(strList *);
void sfRoomGetInfoLine(strList *);
void sfRoomGetName(strList *);
void sfRoomGetNetID(strList *);
void sfRoomGetPGroup(strList *);
void sfRoomIsAnonymous(strList *);
void sfRoomIsApplication(strList *);
void sfRoomIsArchive(strList *);
void sfRoomIsAutoApp(strList *);
void sfRoomIsBIO(strList *);
void sfRoomIsBooleanGroup(strList *);
void sfRoomIsDownloadOnly(strList *);
void sfRoomIsGroupModerates(strList *);
void sfRoomIsGroupOnly(strList *);
void sfRoomIsInuse(strList *);
void sfRoomIsModerated(strList *);
void sfRoomIsMsDOSdir(strList *);
void sfRoomIsPermanent(strList *);
void sfRoomIsPrivilegedGroup(strList *);
void sfRoomIsPublic(strList *);
void sfRoomIsReadOnly(strList *);
void sfRoomIsShared(strList *);
void sfRoomIsSubject(strList *);
void sfRoomIsUnexcludable(strList *);
void sfRoomIsUploadOnly(strList *);
void sfRoomLoad(strList *params);
void sfRoomSave(strList *params);
void sfRoomSetAnonymous(strList *params);
void sfRoomSetApplication(strList *params);
void sfRoomSetApplicationName(strList *params);
void sfRoomSetArchive(strList *params);
void sfRoomSetArchiveFile(strList *params);
void sfRoomSetAutoApp(strList *params);
void sfRoomSetBIO(strList *params);
void sfRoomSetBooleanGroup(strList *params);
void sfRoomSetDescFile(strList *params);
void sfRoomSetDictionary(strList *params);
void sfRoomSetDirectory(strList *params);
void sfRoomSetDownloadOnly(strList *params);
void sfRoomSetGroup(strList *params);
void sfRoomSetGroupModerates(strList *params);
void sfRoomSetGroupOnly(strList *params);
void sfRoomSetInfoLine(strList *params);
void sfRoomSetInuse(strList *params);
void sfRoomSetModerated(strList *params);
void sfRoomSetMsDOSdir(strList *params);
void sfRoomSetName(strList *params);
void sfRoomSetNetID(strList *params);
void sfRoomSetPermanent(strList *params);
void sfRoomSetPGroup(strList *params);
void sfRoomSetPrivilegedGroup(strList *params);
void sfRoomSetPublic(strList *params);
void sfRoomSetReadOnly(strList *params);
void sfRoomSetShared(strList *params);
void sfRoomSetSubject(strList *params);
void sfRoomSetUnexcludable(strList *params);
void sfRoomSetUploadOnly(strList *params);
void sfRoomStart(strList *);
void sfRoomUseCurrentRoom(strList *);
void sfNormalizeString(strList *params);
void sfGetMsgReadOption(strList *params);
void sfSetMsgReadOption(strList *params);
void sfCmdAideAttributes(strList *);
void sfCmdAideCopyMessage(strList *);
void sfCmdAideEditRoom(strList *);
void sfCmdAideFileInfoSet(strList *);
void sfCmdAideGroup(strList *);
void sfCmdAideHall(strList *);
void sfCmdAideInsert(strList *);
void sfCmdAideJumpToAideAndMaintenance(strList *);
void sfCmdAideKillRoom(strList *);
void sfCmdAideListGroup(strList *);
void sfCmdAideMoveFile(strList *);
void sfCmdAideMoveRoom(strList *params);
void sfCmdAideNameMessages(strList *);
void sfCmdAideQueueAutomark(strList *);
void sfCmdAideQueueClear(strList *);
void sfCmdAideQueueInsert(strList *);
void sfCmdAideQueueKill(strList *);
void sfCmdAideQueueList(strList *);
void sfCmdAideQueueMove(strList *);
void sfCmdAideQueueRoomMark(strList *);
void sfCmdAideQueueSort(strList *);
void sfCmdAideRenameFile(strList *);
void sfCmdAideSetFileInfo(strList *);
void sfCmdAideUnlink(strList *);
void sfCmdAideWindow(strList *);
void sfCmdIntro(strList *);
void sfCmdVerboseToggle(strList *);
void sfCmdExpertToggle(strList *);
void sfCmdY(strList *params);
void sfCmdEnterApplication(strList *);
void sfCmdEnterBorder(strList *);
void sfCmdEnterConfiguration(strList *params);
void sfCmdEnterDoor(strList *);
void sfCmdEnterHall(strList *);
void sfCmdEnterName(strList *);
void sfCmdEnterPassword(strList *);
void sfCmdEnterRoom(strList *params);
void sfCmdEnterTextfile(strList *params);
void sfCmdEnterTitleSurname(strList *);
void sfCmdEnterWithProtocol(strList *params);
void sfCmdExcludeRoom(strList *);
void sfCmdFingerDelete(strList *);
void sfCmdFingerEdit(strList *);
void sfCmdFingerList(strList *);
void sfCmdFingerUser(strList *);
void sfCmdInviteRoomList(strList *);
void sfCmdInviteUser(strList *);
void sfCmdInviteUserList(strList *);
void sfCmdTerminateQuitAlso(strList *params);
void sfCmdTerminateStay(strList *);
void sfRoomGetCreator(strList *);
void sfRoomSetCreator(strList *params);
// Documented through here

//docr now allows integer paramter; getVersion 2 is defined
void sfGetSoftVerb(strList *);
void sfSetSoftVerb(strList *params);
void sfGetProgramName(strList *);
void sfGetProgramAuthor(strList *);
void sfGetCompileDate(strList *);
void sfGetCompileTime(strList *);
void sfResetOutParagraph(strList *);
void sfGetUpTime(strList *);
void sfHaveConnection(strList *);
void sfGetConnectionTime(strList *);
void sfGetLoginTime(strList *);

void sfGetUppercaseMsgNym(strList *);
void sfGetUppercaseMsgsNym(strList *);
void sfGetUppercaseRoomNym(strList *);
void sfGetUppercaseRoomsNym(strList *);
void sfGetUppercaseHallNym(strList *);
void sfGetUppercaseHallsNym(strList *);
void sfGetUppercaseUserNym(strList *);
void sfGetUppercaseUsersNym(strList *);
void sfGetUppercaseGroupNym(strList *);
void sfGetUppercaseGroupsNym(strList *);
void sfGetUppercaseCreditNym(strList *);
void sfGetUppercaseCreditsNym(strList *);

void sfGetLowercaseMsgNym(strList *);
void sfGetLowercaseMsgsNym(strList *);
void sfGetLowercaseRoomNym(strList *);
void sfGetLowercaseRoomsNym(strList *);
void sfGetLowercaseHallNym(strList *);
void sfGetLowercaseHallsNym(strList *);
void sfGetLowercaseUserNym(strList *);
void sfGetLowercaseUsersNym(strList *);
void sfGetLowercaseGroupNym(strList *);
void sfGetLowercaseGroupsNym(strList *);
void sfGetLowercaseCreditNym(strList *);
void sfGetLowercaseCreditsNym(strList *);

#ifdef SCRIPTMAIN
internalFunctionRecord far internalFunctions[] =
	{
	{sfAddCommasToNumber},
	{sfAddDupCheck},
	{sfAddNetIDToNetIDCit},
	{sfAdduser},
	{sfAddWatch},
	{sfAppendToNetCommandReply},
	{sfAsciitochar},
	{sfAskyn},
	{sfChangedir},
	{sfChartoascii},
	{sfChat},
	{sfChatreq},
	{sfCheckForDup},
	{sfCheckUserOutControl},
	{sfClearBreakpoint},
	{sfClosefile},
	{sfCmdAideAttributes},
	{sfCmdAideCopyMessage},
	{sfCmdAideEditRoom},
	{sfCmdAideFileInfoSet},
	{sfCmdAideGroup},
	{sfCmdAideHall},
	{sfCmdAideInsert},
	{sfCmdAideJumpToAideAndMaintenance},
	{sfCmdAideKillRoom},
	{sfCmdAideListGroup},
	{sfCmdAideMoveFile},
	{sfCmdAideMoveRoom},
	{sfCmdAideNameMessages},
	{sfCmdAideQueueAutomark},
	{sfCmdAideQueueClear},
	{sfCmdAideQueueInsert},
	{sfCmdAideQueueKill},
	{sfCmdAideQueueList},
	{sfCmdAideQueueMove},
	{sfCmdAideQueueRoomMark},
	{sfCmdAideQueueSort},
	{sfCmdAideRenameFile},
	{sfCmdAideSetFileInfo},
	{sfCmdAideUnlink},
	{sfCmdAideWindow},
	{sfCmdEnterApplication},
	{sfCmdEnterBorder},
	{sfCmdEnterConfiguration},
	{sfCmdEnterDoor},
	{sfCmdEnterHall},
	{sfCmdEnterName},
	{sfCmdEnterPassword},
	{sfCmdEnterRoom},
	{sfCmdEnterTextfile},
	{sfCmdEnterTitleSurname},
	{sfCmdEnterWithProtocol},
	{sfCmdExcludeRoom},
	{sfCmdExpertToggle},
	{sfCmdFingerDelete},
	{sfCmdFingerEdit},
	{sfCmdFingerList},
	{sfCmdFingerUser},
	{sfCmdIntro},
	{sfCmdInviteRoomList},
	{sfCmdInviteUser},
	{sfCmdInviteUserList},
	{sfCmdTerminateQuitAlso},
	{sfCmdTerminateStay},
	{sfCmdVerboseToggle},
	{sfCmdY},
	{sfCountDirectory},
	{sfDebuggingMsg},
	{sfDecToHex},
	{sfDeletefile},
	{sfDial},
	{sfDirectoryGetName},
	{sfDirectoryGetSize},
	{sfDirectoryGetTime},
	{sfDiskfree},
	{sfDoccr},
	{sfDochat},
	{sfDoCR},
	{sfDoNetwork},
	{sfDumpDirectory},
	{sfDumpf},
	{sfDumpfc},
	{sfExitCitadel},
	{sfFileeof},
	{sfFileexists},
	{sfFindHallByName},
	{sfFindNodeByNameOrAddress},
	{sfFindRoomByName},
	{sfFindRoomByNetID},
	{sfFindUserByName},
	{sfGetaccounting},
	{sfGetaidechathour},
	{sfGetaidehall},
	{sfGetaidemoderate},
	{sfGetalias},
	{sfGetanonauthor},
	{sfGetapplpath},
	{sfGetattr},
	{sfGetautoansi},
	{sfGetbattr},
	{sfGetbells},
	{sfGetbios},
	{sfGetborder},
	{sfGetcallno},
	{sfGetcattr},
	{sfGetchaton},
	{sfGetcheckcts},
	{sfGetCit86Country},
	{sfGetCit86Domain},
	{sfGetcolors},
	{sfGetCommentOfNetIDInNetIDCit},
	{sfGetCompileDate},
	{sfGetCompileTime},
	{sfGetConnectionTime},
	{sfGetconnectrate},
	{sfGetconnectwait},
	{sfGetContextSensitiveText},
	{sfGetcountbeep},
	{sfGetcurrenttime},
	{sfGetDebugSystem},
	{sfGetdefdatestamp},
	{sfGetdefnetprefix},
	{sfGetdefrprompt},
	{sfGetdefvdatestamp},
	{sfGetdialmacro},
	{sfGetdialpref},
	{sfGetdialring},
	{sfGetdialsetup},
	{sfGetDirectory},
	{sfGetdirpath},
	{sfGetdiskfree},
	{sfGetdlpath},
	{sfGetdownshift},
	{sfGetdumbmodem},
	{sfGetentername},
	{sfGetenterok},
	{sfGetentersur},
	{sfGetexpire},
	{sfGetf6pass},
	{sfGetforcelogin},
	{sfGetFormattedString},
	{sfGetforwardsys},
	{sfGetfuelbarempty},
	{sfGetfuelbarfull},
	{sfGetgrpdesc},
	{sfGetgrpname},
	{sfGetgroupslot},
	{sfGethalldesc},
	{sfGethallgroup},
	{sfGethallname},
	{sfGethangup},
	{sfGethangupdelay},
	{sfGethelppath},
	{sfGethelppath2},
	{sfGethomepath},
	{sfGetidletimeout},
	{sfGetinitbaud},
	{sfGetlclosedsys},
	{sfGetlcreate},
	{sfGetLocID},
	{sfGetlogextdir},
	{sfGetLoginTime},
	{sfGetLowercaseCreditNym},
	{sfGetLowercaseCreditsNym},
	{sfGetLowercaseGroupNym},
	{sfGetLowercaseGroupsNym},
	{sfGetLowercaseHallNym},
	{sfGetLowercaseHallsNym},
	{sfGetLowercaseMsgNym},
	{sfGetLowercaseMsgsNym},
	{sfGetLowercaseRoomNym},
	{sfGetLowercaseRoomsNym},
	{sfGetLowercaseUserNym},
	{sfGetLowercaseUsersNym},
	{sfGetlquestionnaire},
	{sfGetlsysmsg},
	{sfGetltaide},
	{sfGetltalias},
	{sfGetltcallno},
	{sfGetltexpert},
	{sfGetltinhash},
	{sfGetltinuse},
	{sfGetltlfmask},
	{sfGetltlocid},
	{sfGetltlogslot},
	{sfGetltname},
	{sfGetltnetuser},
	{sfGetltnoaccount},
	{sfGetltnode},
	{sfGetltnomail},
	{sfGetltoldtoo},
	{sfGetltpermanent},
	{sfGetltproblem},
	{sfGetltpwhash},
	{sfGetltroomtell},
	{sfGetltsysop},
	{sfGetlttabs},
	{sfGetltucmask},
	{sfGetltunlisted},
	{sfGetlverified},
	{sfGetmaxborders},
	{sfGetmaxfiles},
	{sfGetmaxgroups},
	{sfGetmaxhalls},
	{sfGetmaxjumpback},
	{sfGetmaxlogtab},
	{sfGetmaxpoop},
	{sfGetmaxrooms},
	{sfGetmcidate},
	{sfGetmcifirstname},
	{sfGetmciname},
	{sfGetmcion},
	{sfGetmcipoop},
	{sfGetmcitime},
	{sfGetmdata},
	{sfGetmessagek},
	{sfGetmessageroom},
	{sfGetminbaud},
	{sfGetModemStr},
	{sfGetmodsetup},
	{sfGetmodunsetup},
	{sfGetmoreprompt},
	{sfGetmsgcompress},
	{sfGetmsgpath},
	{sfGetMsgReadOption},
	{sfGetmsgverb},
	{sfGetmtoldest},
	{sfGetNetCommandText},
	{sfGetNetCommandTextToFile},
	{sfGetnetid},
	{sfGetnetmail},
	{sfGetnetsurnames},
	{sfGetnettitles},
	{sfGetnewest},
	{sfGetnewuserapp},
	{sfGetnmessages},
	{sfGetnodecountry},
	{sfGetnodename},
	{sfGetnodephone},
	{sfGetnoderegion},
	{sfGetnodesig},
	{sfGetnopwecho},
	{sfGetNumberOfMessages},
	{sfGetoffhook},
	{sfGetoffhookstr},
	{sfGetoldcount},
	{sfGetoldest},
	{sfGetOneKey},
	{sfGetparam},
	{sfGetparamcount},
	{sfGetpoop},
	{sfGetpoopuser},
	{sfGetportrate},
	{sfGetprinter},
	{sfGetprinterprompt},
	{sfGetProgramAuthor},
	{sfGetProgramName},
	{sfGetreadluser},
	{sfGetreadok},
	{sfGetrestoremode},
	{sfGetroombypass},
	{sfGetroomdir},
	{sfGetroomentered},
	{sfGetroomgroup},
	{sfGetroomhasmail},
	{sfGetroomhasoldmail},
	{sfGetroommsgs},
	{sfGetroomname},
	{sfGetroomnew},
	{sfGetroomok},
	{sfGetroompath},
	{sfGetroomtotal},
	{sfGetroomvisited},
	{sfGetscreensave},
	{sfGetscrollcolors},
	{sfGetscrollsize},
	{sfGetscrolltimeout},
	{sfGetsetcensor},
	{sfGetsetmsgnym},
	{sfGetsleepcount},
	{sfGetsleepprompt},
	{sfGetSoftVerb},
	{sfGetstatnum},
	{sfGetstr},
	{sfGetsubhubs},
	{sfGetsurnames},
	{sfGetsysborders},
	{sfGetsysopname},
	{sfGettemppath},
	{sfGettimeout},
	{sfGettitles},
	{sfGettranspath},
	{sfGettrapfile},
	{sfGettwirly},
	{sfGettwirlypause},
	{sfGettwitcountry},
	{sfGettwitregion},
	{sfGetunlogbal},
	{sfGetunlogtimeout},
	{sfGetupday},
	{sfGetuphour},
	{sfGetUppercaseCreditNym},
	{sfGetUppercaseCreditsNym},
	{sfGetUppercaseGroupNym},
	{sfGetUppercaseGroupsNym},
	{sfGetUppercaseHallNym},
	{sfGetUppercaseHallsNym},
	{sfGetUppercaseMsgNym},
	{sfGetUppercaseMsgsNym},
	{sfGetUppercaseRoomNym},
	{sfGetUppercaseRoomsNym},
	{sfGetUppercaseUserNym},
	{sfGetUppercaseUsersNym},
	{sfGetUpTime},
	{sfGetuseraddr1},
	{sfGetuseraddr2},
	{sfGetuseraddr3},
	{sfGetusercallno},
	{sfGetusercalltime},
	{sfGetusercredits},
	{sfGetuserdefhall},
	{sfGetuserdstamp},
	{sfGetuserfwd},
	{sfGetuserfwdnode},
	{sfGetuserin},
	{sfGetuserlasthall},
	{sfGetuserlastpointer},
	{sfGetuserlastroom},
	{sfGetuserlines},
	{sfGetusername},
	{sfGetusernetpref},
	{sfGetusernulls},
	{sfGetuserphone},
	{sfGetuserpoop},
	{sfGetuserprompt},
	{sfGetuserproto},
	{sfGetuserpw},
	{sfGetuserrealname},
	{sfGetusersig},
	{sfGetusersurname},
	{sfGetusertitle},
	{sfGetuservdstamp},
	{sfGetuserwidth},
	{sfGetuttr},
	{sfGetVersion},
	{sfGetvmemfile},
	{sfGetwattr},
	{sfHangup},
	{sfHasTermWindow},
	{sfHaveConnection},
	{sfHexToDec},
	{sfHowLongAgoWasIt},
	{sfIchar},
	{sfIcharne},
	{sfIsCanOutput},
	{sfIsConsoleLocked},
	{sfIsFilenameLegal},
	{sfIsgrpautoadd},
	{sfIsgrphidden},
	{sfIsgrpinuse},
	{sfIsgrplocked},
	{sfIshalldesc},
	{sfIshallenterroom},
	{sfIshallgrp},
	{sfIshallinuse},
	{sfIsingroup},
	{sfIsLoggedIn},
	{sfIsMatchWildcard},
	{sfIsNetIDInNetIDCit},
	{sfIsOutputJump},
	{sfIsOutputNext},
	{sfIsOutputStop},
	{sfIsroomanon},
	{sfIsroomapplic},
	{sfIsroombio},
	{sfIsRoomBypassed},
	{sfIsroomdir},
	{sfIsroomdownonly},
	{sfIsroomgrpmod},
	{sfIsroomgrponly},
	{sfIsRoomInHall},
	{sfIsroominuse},
	{sfIsroommoderated},
	{sfIsroomperm},
	{sfIsroomprivgrp},
	{sfIsroompublic},
	{sfIsroomreadonly},
	{sfIsroomshared},
	{sfIsroomsubject},
	{sfIsroomuponly},
	{sfIsroomwindow},
	{sfIsuseraide},
	{sfIsuserdisplayts},
	{sfIsuserentborders},
	{sfIsuserexpert},
	{sfIsuserfwdnode},
	{sfIsuserhalllock},
	{sfIsuserhalltell},
	{sfIsuserhideexcl},
	{sfIsuserinuse},
	{sfIsuserlastold},
	{sfIsuserlf},
	{sfIsuserlocksig},
	{sfIsuserminibin},
	{sfIsusermsgcls},
	{sfIsusermsgpause},
	{sfIsusernet},
	{sfIsusernexthall},
	{sfIsusernoacct},
	{sfIsusernochat},
	{sfIsusernode},
	{sfIsusernodownload},
	{sfIsusernomail},
	{sfIsusernoupload},
	{sfIsuserout300},
	{sfIsuserperm},
	{sfIsuserprint},
	{sfIsuserproblem},
	{sfIsuserpsycho},
	{sfIsuserroominfo},
	{sfIsuserroomtell},
	{sfIsuserseeborders},
	{sfIsusersignatures},
	{sfIsusersubjects},
	{sfIsusersurlock},
	{sfIsusersysop},
	{sfIsusertabs},
	{sfIsusertwirly},
	{sfIsuserunlisted},
	{sfIsuseruponly},
	{sfIsuservcont},
	{sfIsuserverbose},
	{sfIsuserverified},
	{sfIsuserviewcensor},
	{sfKeyboardAdd},
	{sfKeyboardFlush},
	{sfKillRoom},
	{sfKillUser},
	{sfMemoryFree},
	{sfModemflush},
	{sfModeminitport},
	{sfMsgAddComment},
	{sfMsgAppendText},
	{sfMsgClear},
	{sfMsgDump},
	{sfMsgGetAuthor},
	{sfMsgGetCit86Country},
	{sfMsgGetComment},
	{sfMsgGetCommentCount},
	{sfMsgGetCopyOfMessage},
	{sfMsgGetCreationRoom},
	{sfMsgGetCreationTime},
	{sfMsgGetDestinationAddress},
	{sfMsgGetEncryptionKey},
	{sfMsgGetEZCreationTime},
	{sfMsgGetFileLink},
	{sfMsgGetForward},
	{sfMsgGetFromPath},
	{sfMsgGetGroup},
	{sfMsgGetHeadLoc},
	{sfMsgGetLocalID},
	{sfMsgGetOriginAddress},
	{sfMsgGetOriginalAttribute},
	{sfMsgGetOriginalID},
	{sfMsgGetOriginalRoom},
	{sfMsgGetOriginCountry},
	{sfMsgGetOriginNodeName},
	{sfMsgGetOriginPhoneNumber},
	{sfMsgGetOriginRegion},
	{sfMsgGetOriginSoftware},
	{sfMsgGetPublicToUser},
	{sfMsgGetRealName},
	{sfMsgGetReplyToMessage},
	{sfMsgGetRoomNumber},
	{sfMsgGetSignature},
	{sfMsgGetSourceID},
	{sfMsgGetSourceRoomName},
	{sfMsgGetSubject},
	{sfMsgGetSurname},
	{sfMsgGetTitle},
	{sfMsgGetToCountry},
	{sfMsgGetToNodeName},
	{sfMsgGetToPath},
	{sfMsgGetToPhoneNumber},
	{sfMsgGetToRegion},
	{sfMsgGetToUser},
	{sfMsgGetTwitCountry},
	{sfMsgGetTwitRegion},
	{sfMsgGetUserSignature},
	{sfMsgGetX},
	{sfMsgHeaderClear},
	{sfMsgIsCensored},
	{sfMsgIsCompressed},
	{sfMsgIsEncrypted},
	{sfMsgIsLocal},
	{sfMsgIsMadeVisible},
	{sfMsgIsMoreFlag},
	{sfMsgIsReceiptConfirmationRequested},
	{sfMsgIsReceived},
	{sfMsgIsRepliedTo},
	{sfMsgIsViewDuplicate},
	{sfMsgLoad},
	{sfMsgLoadTextFromFile},
	{sfMsgSave},
	{sfMsgSaveTextToFile},
	{sfMsgSetAuthor},
	{sfMsgSetCensored},
	{sfMsgSetCreationRoom},
	{sfMsgSetCreationTime},
	{sfMsgSetDestinationAddress},
	{sfMsgSetEncrypted},
	{sfMsgSetEncryptionKey},
	{sfMsgSetEZCreationTime},
	{sfMsgSetFileLink},
	{sfMsgSetForward},
	{sfMsgSetFromPath},
	{sfMsgSetGroup},
	{sfMsgSetLocal},
	{sfMsgSetMoreFlag},
	{sfMsgSetOriginalRoom},
	{sfMsgSetPublicToUser},
	{sfMsgSetRealName},
	{sfMsgSetReceiptConfirmationRequested},
	{sfMsgSetReplyToMessage},
	{sfMsgSetRoomNumber},
	{sfMsgSetSignature},
	{sfMsgSetSourceRoomName},
	{sfMsgSetSubject},
	{sfMsgSetSurname},
	{sfMsgSetText},
	{sfMsgSetTitle},
	{sfMsgSetToPath},
	{sfMsgSetToPhoneNumber},
	{sfMsgSetToUser},
	{sfMsgSetUserSignature},
	{sfMsgSetX},
	{sfMsgShow},
	{sfMsgStart},
	{sfMsgTabGetAuthHash},
	{sfMsgTabGetCopyOffset},
	{sfMsgTabGetLocation},
	{sfMsgTabGetNextRoomMsg},
	{sfMsgTabGetOriginID},
	{sfMsgTabGetPrevRoomMsg},
	{sfMsgTabGetRoomNum},
	{sfMsgTabGetToHash},
	{sfMsgTabIsCensored},
	{sfMsgTabIsCopy},
	{sfMsgTabIsForwarded},
	{sfMsgTabIsInuse},
	{sfMsgTabIsLimited},
	{sfMsgTabIsLocal},
	{sfMsgTabIsMadevis},
	{sfMsgTabIsMail},
	{sfMsgTabIsMassemail},
	{sfMsgTabIsModerated},
	{sfMsgTabIsNetMail},
	{sfMsgTabIsNetworked},
	{sfMsgTabIsProblem},
	{sfMsgTabIsReceived},
	{sfMsgTabIsRepliedTo},
	{sfNet69fetch},
	{sfNet69incorporate},
	{sfNet69RoomReq},
	{sfNet86fetch},
	{sfNet86incorporate},
	{sfNormalizeString},
	{sfOpenfile},
	{sfOutstrpaced},
	{sfParseDateString},
	{sfParseTimeString},
	{sfPause},
	{sfPauseScript},
	{sfPlaysound},
	{sfPrintc},
	{sfPrintm},
	{sfRandom},
	{sfReadAplOnExit},
	{sfReadbool},
	{sfReadChar},
	{sfReadint},
	{sfReadlong},
	{sfReadstr},
	{sfReaduint},
	{sfReadulong},
	{sfRemoveNetIDFromNetIDCit},
	{sfRemoveuser},
	{sfRemoveWatch},
	{sfRenamefile},
	{sfResetOutParagraph},
	{sfRoomChangeDir},
	{sfRoomClear},
	{sfRoomDump},
	{sfRoomGetApplicationName},
	{sfRoomGetArchiveFile},
	{sfRoomGetCreator},
	{sfRoomGetDescFile},
	{sfRoomGetDictionary},
	{sfRoomGetDirectory},
	{sfRoomGetGroup},
	{sfRoomGetInfoLine},
	{sfRoomGetName},
	{sfRoomGetNetID},
	{sfRoomGetPGroup},
	{sfRoomIsAnonymous},
	{sfRoomIsApplication},
	{sfRoomIsArchive},
	{sfRoomIsAutoApp},
	{sfRoomIsBIO},
	{sfRoomIsBooleanGroup},
	{sfRoomIsDownloadOnly},
	{sfRoomIsGroupModerates},
	{sfRoomIsGroupOnly},
	{sfRoomIsInuse},
	{sfRoomIsModerated},
	{sfRoomIsMsDOSdir},
	{sfRoomIsPermanent},
	{sfRoomIsPrivilegedGroup},
	{sfRoomIsPublic},
	{sfRoomIsReadOnly},
	{sfRoomIsShared},
	{sfRoomIsSubject},
	{sfRoomIsUnexcludable},
	{sfRoomIsUploadOnly },
	{sfRoomLoad},
	{sfRoomSave},
	{sfRoomSetAnonymous},
	{sfRoomSetApplication},
	{sfRoomSetApplicationName},
	{sfRoomSetArchive},
	{sfRoomSetArchiveFile},
	{sfRoomSetAutoApp},
	{sfRoomSetBIO},
	{sfRoomSetBooleanGroup},
	{sfRoomSetCreator},
	{sfRoomSetDescFile},
	{sfRoomSetDictionary},
	{sfRoomSetDirectory},
	{sfRoomSetDownloadOnly},
	{sfRoomSetGroup},
	{sfRoomSetGroupModerates},
	{sfRoomSetGroupOnly},
	{sfRoomSetInfoLine},
	{sfRoomSetInuse},
	{sfRoomSetModerated},
	{sfRoomSetMsDOSdir},
	{sfRoomSetName},
	{sfRoomSetNetID},
	{sfRoomSetPermanent},
	{sfRoomSetPGroup},
	{sfRoomSetPrivilegedGroup},
	{sfRoomSetPublic},
	{sfRoomSetReadOnly},
	{sfRoomSetShared},
	{sfRoomSetSubject},
	{sfRoomSetUnexcludable},
	{sfRoomSetUploadOnly },
	{sfRoomStart},
	{sfRoomUseCurrentRoom},
	{sfRunapplic},
	{sfSayascii},
	{sfSeekfile},
	{sfSetaidechathour},
	{sfSetaidehall},
	{sfSetaidemoderate},
	{sfSetanonauthor},
	{sfSetapplpath},
	{sfSetattr},
	{sfSetautoansi},
	{sfSetbattr},
	{sfSetbells},
	{sfSetborder},
	{sfSetBreakpoint},
	{sfSetcattr},
	{sfSetchaton},
	{sfSetcheckcts},
	{sfSetCit86Country},
	{sfSetCit86Domain},
	{sfSetcolors},
	{sfSetconnectwait},
	{sfSetConsoleLock},
	{sfSetcountbeep},
	{sfSetcreditnym},
	{sfSetcreditsnym},
	{sfSetDebugScript},
	{sfSetDebugSystem},
	{sfSetdefdatestamp},
	{sfSetdefnetprefix},
	{sfSetdefrprompt},
	{sfSetdefvdatestamp},
	{sfSetdialmacro},
	{sfSetdialpref},
	{sfSetdialring},
	{sfSetdialsetup},
	{sfSetdirpath},
	{sfSetdiskfree},
	{sfSetdlpath},
	{sfSetdownshift},
	{sfSetdumbmodem},
	{sfSetentername},
	{sfSetenterok},
	{sfSetentersur},
	{sfSetexpire},
	{sfSetf6pass},
	{sfSetforcelogin},
	{sfSetforwardsys},
	{sfSetfuelbarempty},
	{sfSetfuelbarfull},
	{sfSethangup},
	{sfSethangupdelay},
	{sfSethelppath},
	{sfSethelppath2},
	{sfSethomepath},
	{sfSetidletimeout},
	{sfSetinitbaud},
	{sfSetlclosedsys},
	{sfSetlcreate},
	{sfSetlogextdir},
	{sfSetlsysmsg},
	{sfSetlverified},
	{sfSetmaxfiles},
	{sfSetmcidate},
	{sfSetmcifirstname},
	{sfSetmciname},
	{sfSetmcion},
	{sfSetmcipoop},
	{sfSetmcitime},
	{sfSetmdata},
	{sfSetmessageroom},
	{sfSetminbaud},
	{sfSetmodsetup},
	{sfSetmodunsetup},
	{sfSetmoreprompt},
	{sfSetmsgcompress},
	{sfSetmsgnym},
	{sfSetmsgpath},
	{sfSetMsgReadOption},
	{sfSetmsgsnym},
	{sfSetmsgverb},
	{sfSetnetmail},
	{sfSetnetsurnames},
	{sfSetnettitles},
	{sfSetnewuserapp},
	{sfSetnodecountry},
	{sfSetnodename},
	{sfSetnodephone},
	{sfSetnoderegion},
	{sfSetnopwecho},
	{sfSetoffhook},
	{sfSetoffhookstr},
	{sfSetoldcount},
	{sfSetOutImpervious},
	{sfSetOutNormal},
	{sfSetOutNostop},
	{sfSetpoop},
	{sfSetportrate},
	{sfSetprinter},
	{sfSetprinterprompt},
	{sfSetRandom},
	{sfSetreadluser},
	{sfSetreadok},
	{sfSetrestoremode},
	{sfSetRoomBypassed},
	{sfSetRoomInHall},
	{sfSetroomok},
	{sfSetroompath},
	{sfSetscreensave},
	{sfSetscrolltimeout},
	{sfSetsetcensor},
	{sfSetsetmsgnym},
	{sfSetsleepcount},
	{sfSetsleepprompt},
	{sfSetSoftVerb},
	{sfSetsubhubs},
	{sfSetsurnames},
	{sfSetsysopname},
	{sfSettemppath},
	{sfSettimeout},
	{sfSettitles},
	{sfSettranspath},
	{sfSettrapfile},
	{sfSettwirly},
	{sfSettwirlypause},
	{sfSettwitcountry},
	{sfSettwitregion},
	{sfSetunlogbal},
	{sfSetunlogtimeout},
	{sfSetupday},
	{sfSetuphour},
	{sfSetusercredits},
	{sfSetuttr},
	{sfSetwattr},
	{sfSformat},
	{sfStartDupCheck},
	{sfStartMessageEditor},
	{sfStartDupCheck},		// StopDupCheck (uses same code as StartDupCheck)
	{sfStrftime},
	{sfStringIsAddress},
	{sfStrPos},
	{sfTellfile},
	{sfThisroom},
	{sfUserAddKillNode},
	{sfUserAddKillRegion},
	{sfUserAddKillText},
	{sfUserAddKillUser},
	{sfUserAddWordToDictionary},
	{sfUserClear},
	{sfUserDump},
	{sfUserGetAlias},
	{sfUserGetBirthDate},
	{sfUserGetCallLimit},
	{sfUserGetCallNumber},
	{sfUserGetCallsToday},
	{sfUserGetCallTime},
	{sfUserGetCredits},
	{sfUserGetDateStamp},
	{sfUserGetDefaultHall},
	{sfUserGetDefaultProtocol},
	{sfUserGetDefaultRoom},
	{sfUserGetDL_Bytes},
	{sfUserGetDL_Num},
	{sfUserGetFirstOn},
	{sfUserGetForwardAddr},
	{sfUserGetForwardAddrNode},
	{sfUserGetForwardAddrRegion},
	{sfUserGetInitials},
	{sfUserGetKillNode},
	{sfUserGetKillRegion},
	{sfUserGetKillText},
	{sfUserGetKillUser},
	{sfUserGetLastHall},
	{sfUserGetLastMessage},
	{sfUserGetLastRoom},
	{sfUserGetLinesPerScreen},
	{sfUserGetLocID},
	{sfUserGetLogins},
	{sfUserGetMailAddr1},
	{sfUserGetMailAddr2},
	{sfUserGetMailAddr3},
	{sfUserGetMorePrompt},
	{sfUserGetName},
	{sfUserGetNetPrefix},
	{sfUserGetNulls},
	{sfUserGetNumKillNode},
	{sfUserGetNumKillRegion},
	{sfUserGetNumKillText},
	{sfUserGetNumKillUser},
	{sfUserGetNumUserShow},
	{sfUserGetOccupation},
	{sfUserGetPassword},
	{sfUserGetPasswordChangeTime},
	{sfUserGetPhoneNumber},
	{sfUserGetPoopcount},
	{sfUserGetPosted},
	{sfUserGetPromptFormat},
	{sfUserGetRead},
	{sfUserGetRealName},
	{sfUserGetRoomNewPointer},
	{sfUserGetSex},
	{sfUserGetSignature},
	{sfUserGetSpellCheckMode},
	{sfUserGetSurname},
	{sfUserGetTitle},
	{sfUserGetTotalTime},
	{sfUserGetUL_Bytes},
	{sfUserGetUL_Num},
	{sfUserGetUserDefined},
	{sfUserGetVerboseDateStamp},
	{sfUserGetWhereHear},
	{sfUserGetWidth},
	{sfUserIsAccounting},
	{sfUserIsAide},
	{sfUserIsAutoNextHall},
	{sfUserIsAutoVerbose},
	{sfUserIsChat},
	{sfUserIsCheckAllCaps},
	{sfUserIsCheckApostropheS},
	{sfUserIsCheckDigits},
	{sfUserIsClearScreenBetweenMessages},
	{sfUserIsConfirmNoEO},
	{sfUserIsDefaultHallLocked},
	{sfUserIsDownload},
	{sfUserIsDungeoned},
	{sfUserIsEnterBorders},
	{sfUserIsExcludeEncryptedMessages},
	{sfUserIsExpert},
	{sfUserIsForwardToNode},
	{sfUserIsHideMessageExclusions},
	{sfUserIsIBMRoom},
	{sfUserIsInGroup},
	{sfUserIsInRoom},
	{sfUserIsInuse},
	{sfUserIsKillNode},
	{sfUserIsKillRegion},
	{sfUserIsKillText},
	{sfUserIsKillUser},
	{sfUserIsLinefeeds},
	{sfUserIsMail},
	{sfUserIsMainSysop},
	{sfUserIsMakeRoom},
	{sfUserIsMinibin},
	{sfUserIsMusic},
	{sfUserIsNetUser},
	{sfUserIsNode},
	{sfUserIsOldToo},
	{sfUserIsOut300},
	{sfUserIsPauseBetweenMessages},
	{sfUserIsPermanent},
	{sfUserIsPrintFile},
	{sfUserIsProblem},
	{sfUserIsPsycho},
	{sfUserIsPUnPauses},
	{sfUserIsRoman},
	{sfUserIsRoomExcluded},
	{sfUserIsRoomInPersonalHall},
	{sfUserIsSuperSysop},
	{sfUserIsSurnameLocked},
	{sfUserIsSysop},
	{sfUserIsTabs},
	{sfUserIsTwirly},
	{sfUserIsUnlisted},
	{sfUserIsUpload},
	{sfUserIsUpperOnly},
	{sfUserIsUsePersonalHall},
	{sfUserIsUserDefined},
	{sfUserIsUserSignatureLocked},
	{sfUserIsVerboseLogOut},
	{sfUserIsVerified},
	{sfUserIsViewBorders},
	{sfUserIsViewCensoredMessages},
	{sfUserIsViewCommas},
	{sfUserIsViewHallDescription},
	{sfUserIsViewRoomDesc},
	{sfUserIsViewRoomInfoLines},
	{sfUserIsViewSignatures},
	{sfUserIsViewSubjects},
	{sfUserIsViewTitleSurname},
	{sfUserIsWideRoom},
	{sfUserIsWordInDictionary},
	{sfUserIsYouAreHere},
	{sfUserLoad},
	{sfUserRemoveKillNode},
	{sfUserRemoveKillRegion},
	{sfUserRemoveKillText},
	{sfUserRemoveKillUser},
	{sfUserRemoveUserDefined},
	{sfUserRemoveWordFromDictionary},
	{sfUserSave},
	{sfUserSetAccounting},
	{sfUserSetAide},
	{sfUserSetAlias},
	{sfUserSetAutoNextHall},
	{sfUserSetAutoVerbose},
	{sfUserSetBirthDate},
	{sfUserSetCallLimit},
	{sfUserSetCallNumber},
	{sfUserSetCallsToday},
	{sfUserSetCallTime},
	{sfUserSetChat},
	{sfUserSetCheckAllCaps},
	{sfUserSetCheckApostropheS},
	{sfUserSetCheckDigits},
	{sfUserSetClearScreenBetweenMessages},
	{sfUserSetConfirmNoEO},
	{sfUserSetCredits},
	{sfUserSetDateStamp},
	{sfUserSetDefaultHall},
	{sfUserSetDefaultHallLocked},
	{sfUserSetDefaultProtocol},
	{sfUserSetDefaultRoom},
	{sfUserSetDL_Bytes},
	{sfUserSetDL_Num},
	{sfUserSetDownload},
	{sfUserSetDungeoned},
	{sfUserSetEnterBorders},
	{sfUserSetExcludeEncryptedMessages},
	{sfUserSetExpert},
	{sfUserSetFirstOn},
	{sfUserSetForwardAddr},
	{sfUserSetForwardAddrNode},
	{sfUserSetForwardAddrRegion},
	{sfUserSetForwardToNode},
	{sfUserSetHideMessageExclusions},
	{sfUserSetIBMRoom},
	{sfUserSetInGroup},
	{sfUserSetInitials},
	{sfUserSetInRoom},
	{sfUserSetInuse},
	{sfUserSetLastHall},
	{sfUserSetLastMessage},
	{sfUserSetLastRoom},
	{sfUserSetLinefeeds},
	{sfUserSetLinesPerScreen},
	{sfUserSetLocID},
	{sfUserSetLogins},
	{sfUserSetMail},
	{sfUserSetMailAddr1},
	{sfUserSetMailAddr2},
	{sfUserSetMailAddr3},
	{sfUserSetMakeRoom},
	{sfUserSetMinibin},
	{sfUserSetMorePrompt},
	{sfUserSetMusic},
	{sfUserSetName},
	{sfUserSetNetPrefix},
	{sfUserSetNetUser},
	{sfUserSetNode},
	{sfUserSetNulls},
	{sfUserSetNumUserShow},
	{sfUserSetOccupation},
	{sfUserSetOldToo},
	{sfUserSetOut300},
	{sfUserSetPassword},
	{sfUserSetPasswordChangeTime},
	{sfUserSetPauseBetweenMessages},
	{sfUserSetPermanent},
	{sfUserSetPhoneNumber},
	{sfUserSetPoopcount},
	{sfUserSetPosted},
	{sfUserSetPrintFile},
	{sfUserSetProblem},
	{sfUserSetPromptFormat},
	{sfUserSetPsycho},
	{sfUserSetPUnPauses},
	{sfUserSetRead},
	{sfUserSetRealName},
	{sfUserSetRoman},
	{sfUserSetRoomExcluded},
	{sfUserSetRoomInPersonalHall},
	{sfUserSetRoomNewPointer},
	{sfUserSetSex},
	{sfUserSetSignature},
	{sfUserSetSpellCheckMode},
	{sfUserSetSuperSysop},
	{sfUserSetSurname},
	{sfUserSetSurnameLocked},
	{sfUserSetSysop},
	{sfUserSetTabs},
	{sfUserSetTitle},
	{sfUserSetTotalTime},
	{sfUserSetTwirly},
	{sfUserSetUL_Bytes},
	{sfUserSetUL_Num},
	{sfUserSetUnlisted},
	{sfUserSetUpload},
	{sfUserSetUpperOnly},
	{sfUserSetUsePersonalHall},
	{sfUserSetUserDefined},
	{sfUserSetUserSignatureLocked},
	{sfUserSetVerboseDateStamp},
	{sfUserSetVerboseLogOut},
	{sfUserSetVerified},
	{sfUserSetViewBorders},
	{sfUserSetViewCensoredMessages},
	{sfUserSetViewCommas},
	{sfUserSetViewHallDescription},
	{sfUserSetViewRoomDesc},
	{sfUserSetViewRoomInfoLines},
	{sfUserSetViewSignatures},
	{sfUserSetViewSubjects},
	{sfUserSetViewTitleSurname},
	{sfUserSetWhereHear},
	{sfUserSetWideRoom},
	{sfUserSetWidth},
	{sfUserSetYouAreHere},
	{sfUserStart},
	{sfUserUseCurrentUser},
	{sfWaitfor},
	{sfWriteapl},
	{sfWritebool},
	{sfWriteChar},
	{sfWriteint},
	{sfWritelong},
	{sfWritestr},
	{sfWriteuint},
	{sfWriteulong},
#ifndef NDEBUG
	{NULL},
#endif
	};
#else
extern internalFunctionRecord far internalFunctions[];
#endif
