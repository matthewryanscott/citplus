// --------------------------------------------------------------------------
// Citadel: Group.CPP
//
// Group code

#include "ctdl.h"
#pragma hdrstop

#include "log.h"
#include "group.h"


// --------------------------------------------------------------------------
// Contents
//
// groupexists()			returns # of named group, else CERROR
// groupseeshall()			indicates if group can see hall #
// ingroup()				returns TRUE if log is in named group
// partialgroup()			returns slot of partially named group


GroupBuffer GroupData;			// Group data goes here


// --------------------------------------------------------------------------
// groupexists(): Return # of named group, else CERROR.

g_slot FindGroupByName(const char *TestName)
	{
	for (g_slot i = 0; i < cfg.maxgroups; i++)
		{
		if (GroupData[i].IsInuse() && GroupData[i].IsSameName(TestName))
			{
			return (i);
			}
		}

	return (CERROR);
	}


// --------------------------------------------------------------------------
// partialgroup(): Returns slot # of partial group name, else error.

g_slot TERMWINDOWMEMBER FindGroupByPartialName(const char *TestName, Bool IgnoreInGroup)
	{
	if (strlen(deansi(TestName)) > LABELSIZE)
		{
		return (CERROR);
		}

	g_slot i = FindGroupByName(TestName);
	if (i != CERROR && (IgnoreInGroup || CurrentUser->IsAide() || CurrentUser->IsInGroup(i)))
		{
		return (i);
		}

	label da_gn;
	strcpy(da_gn, deansi(TestName));
	const int length = strlen(da_gn);

	if (!length)
		{
		return (CERROR);
		}

	// Start of string match
	for (i = 0; i < cfg.maxgroups; i++)
		{
		if (GroupData[i].IsInuse())
			{
			label Buffer;
			if ((strnicmp(deansi(GroupData[i].GetName(Buffer, sizeof(Buffer))), da_gn, length) == SAMESTRING) &&
					(IgnoreInGroup || CurrentUser->IsAide() || CurrentUser->IsInGroup(i)))
				{
				return (i);
				}
			}
		}

	// Partial match
	for (i = 0; i < cfg.maxgroups; i++)
		{
		if (GroupData[i].IsInuse())
			{
			label Buffer;
			if (IsSubstr(GroupData[i].GetName(Buffer, sizeof(Buffer)), da_gn) &&
					(IgnoreInGroup || CurrentUser->IsAide() || CurrentUser->IsInGroup(i)))
				{
				return (i);
				}
			}
		}

	return (CERROR);
	}
