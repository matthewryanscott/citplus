// --------------------------------------------------------------------------
// Citadel: NetCmd.CPP
//
// Network command processor.


#include "ctdl.h"
#pragma hdrstop

#include "room.h"
#include "log.h"
#include "group.h"
#include "net.h"
#include "hall.h"
#include "filerdwr.h"
#include "netcmd.h"

// --------------------------------------------------------------------------
// Contents

//	readNetCmdTmp() 	Read and process NETCMD.TMP


const char *netcmdTmp = "NETCMD.TMP";

void TERMWINDOWMEMBER AppendLineToNetCommandReply(const char *str)
	{
	if (NetCmdReplyMsg)
		{
		NetCmdReplyMsg->AppendText(str);
		NetCmdReplyMsg->AppendText(bn);
		}
	}


// --------------------------------------------------------------------------
// readNetCmdTmp(): Read and process NETCMD.TMP.

Bool NetCmdFileC::SaveByte(char Byte)
	{
	assert(CmdFile);
	return (fputc(Byte, CmdFile) != EOF);
	}

Bool NetCmdFileC::SaveString(const char *String)
	{
	assert(CmdFile);
	assert(String);

	return (PutStr(CmdFile, String));
	}

Bool NetCmdFileC::GetByte(char *Byte)
	{
	assert(CmdFile);

	if (feof(CmdFile))
		{
		*Byte = 0;
		return (FALSE);
		}
	else 
		{
		*Byte = (char) fgetc(CmdFile);
		return (TRUE);
		}
	}

void TERMWINDOWMEMBER readNetCmdTmp(ModemConsoleE W)
	{
	assert(NetCommandMsg == NULL);
	assert(NetCmdReplyMsg == NULL);

	NetCommandMsg = new Message;
	NetCmdReplyMsg = new Message;

	if (NetCommandMsg && NetCmdReplyMsg)
		{
		discardable *d;

		if ((d = readData(12)) != NULL)
			{
			char FileName[128];
			FILE *fl;
			const char **ncmsg = (const char **) d->aux;

			sprintf(FileName, sbs, LocalTempPath, netcmdTmp);
			if ((fl = fopen(FileName, FO_RB)) != NULL)
				{
				char **netkeywords = (char **) d->next->aux;
				wDoCR(W);
				wPrintf(W, ncmsg[1]);
				wDoCR(W);

				NetCmdFileC NetCmdFile(fl);

				while (NetCmdFile.LoadAll(NetCommandMsg) == RMS_OK)
					{
					NetCmdReplyMsg->ClearAll();

					if (*NetCommandMsg->GetSubject() == '*')
						{
						// this is a response
						}
					else
						{
						// this is the originating message
						normalizeString(NetCommandMsg->GetTextPointer());

						int i;
						for (i = 0; i < NC_NUM; i++)
							{
							if (SameString(NetCommandMsg->GetSubject(), netkeywords[i]))
								{
								break;
								}
							}

						switch (i)
							{
							case NC_DEBUG:
								{
								DoDebugCmd(ncmsg);
								break;
								}

							case NC_ROOMINFO:
								{
								r_slot rm;

								rm = IdExists(NetCommandMsg->GetText(), TRUE);

								if (rm != CERROR)
									{
									CreateRoomSummary(NetCmdReplyMsg->GetTextPointer(), rm);
									}
								else
									{
									char str[128];
									sprintf(str, getmsg(368), cfg.Lroom_nym, NetCommandMsg->GetText());

									NetCmdReplyMsg->SetText(str);
									}

								break;
								}

							case NC_NODELIST:
								{
								for (l_slot lg = 0; lg < cfg.MAXLOGTAB; lg++)
									{
									if (LTab(lg).IsInuse() &&
											LTab(lg).IsNode())
										{
										char str[80];
										label Buffer, Buffer1, Buffer2;

										sprintf(str, ncmsg[39], LTab(lg).GetAlias(Buffer, sizeof(Buffer)),
												LTab(lg).GetLocID(Buffer1, sizeof(Buffer1)),
												LTab(lg).GetName(Buffer2, sizeof(Buffer2)), bn);

										NetCmdReplyMsg->AppendText(str);
										}
									}

								break;
								}

							case NC_NODEINFO:
								{
								int lg;

								lg = nodexists(NetCommandMsg->GetText());

								if (lg == CERROR)
									{
									char str[128];
									sprintf(str, ncmsg[46], NetCmdReplyMsg->GetText());
									NetCmdReplyMsg->SetText(str);
									}
								else
									{
									NodesCitC *nd = NULL;

#ifdef WINCIT
									if (ReadNodesCit(this, &nd, NetCmdReplyMsg->GetText(), NOMODCON, TRUE))
#else
                                    if (ReadNodesCit(&nd, NetCmdReplyMsg->GetText(), NOMODCON, TRUE))
#endif
										{
										label Buffer;
										char str[128];

										sprintf(str, ncmsg[48], NetCmdReplyMsg->GetText());
										NetCmdReplyMsg->SetText(str);
										NetCmdReplyMsg->AppendText(bn);

										sprintf(str, ncmsg[49], nd->GetName());
										AppendLineToNetCommandReply(str);

										sprintf(str, ncmsg[50], nd->GetOldRegion());
										AppendLineToNetCommandReply(str);

										sprintf(str, ncmsg[51], nd->GetAlias(),
												nd->GetLocID());
										AppendLineToNetCommandReply(str);

										sprintf(str, ncmsg[52], bauds[nd->GetBaud()]);
										AppendLineToNetCommandReply(str);

										sprintf(str, ncmsg[53], nd->GetDialTimeout());
										AppendLineToNetCommandReply(str);

										sprintf(str, ncmsg[54], nd->GetProtocol());
										AppendLineToNetCommandReply(str);

										sprintf(str, ncmsg[55], nd->GetWaitTimeout());
										AppendLineToNetCommandReply(str);

										sprintf(str, ncmsg[56], nd->GetMailFileName());
										AppendLineToNetCommandReply(str);

										sprintf(str, ncmsg[57], nd->GetAutoHall() >= 0 ?
												HallData[nd->GetAutoHall()].GetName(Buffer, sizeof(Buffer)) : ncmsg[74]);
										AppendLineToNetCommandReply(str);

										sprintf(str, ncmsg[58], nd->GetAutoGroup() >= 0 ?
												GroupData[nd->GetAutoGroup()].GetName(Buffer, sizeof(Buffer)) : ncmsg[74]);
										AppendLineToNetCommandReply(str);

										sprintf(str, ncmsg[59],
												GroupData[nd->GetMapUnknownGroup()].GetName(Buffer, sizeof(Buffer)));
										AppendLineToNetCommandReply(str);

										sprintf(str, ncmsg[60], nd->GetNetworkType());
										AppendLineToNetCommandReply(str);

										sprintf(str, ncmsg[61], nd->GetCreatePacket());
										AppendLineToNetCommandReply(str);

										sprintf(str, ncmsg[62], nd->GetExtractPacket());
										AppendLineToNetCommandReply(str);

										sprintf(str, ncmsg[63], nd->IsAutoRoom());
										AppendLineToNetCommandReply(str);

//										sprintf(str, ncmsg[64], nd->GetVerbose());
//										AppendLineToNetCommandReply(str);

										sprintf(str, ncmsg[65], nd->GetRequest());
										AppendLineToNetCommandReply(str);

										sprintf(str, ncmsg[66], nd->IsGateway());
										AppendLineToNetCommandReply(str);

										sprintf(str, ncmsg[67], nd->GetFetch());
										AppendLineToNetCommandReply(str);

										sprintf(str, ncmsg[68], nd->GetNetFail());
										AppendLineToNetCommandReply(str);

										sprintf(str, ncmsg[69], nd->GetRedial());
										AppendLineToNetCommandReply(str);
										}
									else
										{
										NetCmdReplyMsg->SetTextWithFormat(ncmsg[47], NetCommandMsg->GetText());
										}

									freeNode(&nd);
									}

								break;
								}

							case NC_POOPCOUNT:
								{
								l_slot lg;

								lg = FindPersonByName(NetCommandMsg->GetText());

								if (lg != CERROR)
									{
									LogEntry1 Log1;

									if (Log1.Load(LTab(lg).GetLogIndex()))
										{
										label Buffer;
										NetCmdReplyMsg->SetTextWithFormat(ncmsg[70], Log1.GetName(Buffer, sizeof(Buffer)),
												Log1.GetPoopcount());
										}
									else
										{
										label Buffer;
										NetCmdReplyMsg->SetTextWithFormat(ncmsg[73],
												LTab(lg).GetName(Buffer, sizeof(Buffer)));
										}
									}
								else
									{
									NetCmdReplyMsg->SetTextWithFormat(getmsg(595), NetCmdReplyMsg->GetText());
									}

								break;
								}

							case NC_POOPUSER:
								{
								NetCmdReplyMsg->SetTextWithFormat(ncmsg[71], cfg.poopuser, cfg.maxpoop);
								break;
								}

							case NC_FINGER:
								{
								l_slot lg;

								lg = FindPersonByName(NetCommandMsg->GetText());

								if (lg != CERROR)
									{
									LogExtensions LE(0);

									if (LE.Load(LTab(lg).GetLogIndex()))
										{
										char *Buffer = new char[MAXTEXT];
										if (Buffer)
											{
											if (LE.GetFinger(Buffer, MAXTEXT))
												{
												label Buffer1;
												NetCmdReplyMsg->SetTextWithFormat(ncmsg[72],
														LTab(lg).GetName(Buffer1, sizeof(Buffer1)));
												NetCmdReplyMsg->AppendText(bn);
												NetCmdReplyMsg->AppendText(Buffer);
												}
											else
												{
												label Buffer1;
												NetCmdReplyMsg->SetTextWithFormat(getmsg(373),
														LTab(lg).GetName(Buffer1, sizeof(Buffer1)));
												}

											delete [] Buffer;
											}
										}
									else
										{
										label Buffer;
										NetCmdReplyMsg->SetTextWithFormat(ncmsg[73],
												LTab(lg).GetName(Buffer, sizeof(Buffer)));
										}
									}
								else
									{
									NetCmdReplyMsg->SetTextWithFormat(getmsg(595), NetCmdReplyMsg->GetText());
									}

								break;
								}

							default:
								{
								pairedStrings *theCmd;
								for (theCmd = netCmdList; theCmd; theCmd = (pairedStrings *) getNextLL(theCmd))
									{
									if (SameString(NetCommandMsg->GetSubject() + 1, theCmd->string1))
										{
										break;
										}
									}

								if (theCmd)
									{
#ifdef WINCIT
									runScript(theCmd->string2, this);
#else
									runScript(theCmd->string2);
#endif
									}
								else
									{
									NetCmdReplyMsg->SetTextWithFormat(ncmsg[2], NetCommandMsg->GetSubject());
									}

								break;
								}
							}

						NetCmdReplyMsg->SetCreationRoom(MAILROOM);
						NetCmdReplyMsg->SetRoomNumber(MAILROOM);

						NetCmdReplyMsg->SetToUser(NetCommandMsg->GetAuthor());
						NetCmdReplyMsg->SetAuthor(cfg.nodeTitle);

						char Subject[128];
						sprintf(Subject, ncmsg[3], NetCommandMsg->GetSubject());
						Subject[80] = 0;
						NetCmdReplyMsg->SetSubject(Subject);

						if (*NetCommandMsg->GetSourceID())
							{
							label Buffer;

							NetCmdReplyMsg->SetToNodeName(NetCommandMsg->GetOriginNodeName());
							NetCmdReplyMsg->SetToRegion(NetCommandMsg->GetOriginRegion());
							NetCmdReplyMsg->SetToCountry(NetCommandMsg->GetOriginCountry());
							NetCmdReplyMsg->SetDestinationAddress(NetCommandMsg->GetOriginAddress(Buffer));

							if (!save_mail(NetCmdReplyMsg, TRUE, ncmsg[75], CERROR))
								{
								wPrintf(W, ncmsg[4]);
								wDoCR(W);
								}
							}
						}
					}

				fclose(fl);
				unlink(FileName);
				wDoCR(W);
				}

			discardData(d);
			}
		else
			{
			cOutOfMemory(95);
			}
		}
	else
		{
		cOutOfMemory(95);
		}

	delete NetCommandMsg;
	delete NetCmdReplyMsg;

	NetCmdReplyMsg = NULL;
	NetCommandMsg = NULL;
	}
