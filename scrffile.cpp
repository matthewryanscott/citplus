// --------------------------------------------------------------------------
// Citadel: ScrFFile.CPP
//
// File-related internal script functions.

#include "ctdl.h"
#pragma hdrstop

#include "scrfunc.h"
#include "filerdwr.h"


void sfOpenfile(strList *params)
    {
    int i = evaluateInt(params->string);

    if (i >= 0 && i < MAXSCRIPTFILES)
        {
        const char *name = evaluateString(params->next->string);

        if (pCurScript->UsrFile[i])
            {
            fclose(pCurScript->UsrFile[i]);
            }

        pCurScript->UsrFile[i] = fopen(name, FO_RPB);

        if (!pCurScript->UsrFile[i])
            {
            pCurScript->UsrFile[i] = fopen(name, FO_WPB);
            }

        SETBOOL(pCurScript->UsrFile[i] != NULL);
        }
    else
        {
        SETBOOL(FALSE);
        }
    }

void sfClosefile(strList *params)
    {
    int i = evaluateInt(params->string);

    if (pCurScript->UsrFile[i])
        {
        fclose(pCurScript->UsrFile[i]);
        pCurScript->UsrFile[i] = NULL;
        }
    }

void sfDeletefile(strList *params)
    {
    SETBOOL(unlink(evaluateString(params->string)) != -1);
    }

void sfFileeof(strList *params)
    {
    SETBOOL(feof(pCurScript->UsrFile[evaluateInt(params->string)]));
    }

void sfReadstr(strList *params)
    {
    char str[256];

    net69_GetStr(pCurScript->UsrFile[evaluateInt(params->string)], str, 256);
    SETSTR(str);
    }

void sfReadint(strList *params)
    {
    short i;

    fread(&i, sizeof(i), 1, pCurScript->UsrFile[evaluateInt(params->string)]);
    SETINT(i);
    }

void sfReaduint(strList *params)
    {
    ushort ui;

    fread(&ui, sizeof(ui), 1, pCurScript->UsrFile[evaluateInt(params->string)]);
    SETUINT(ui);
    }

void sfReadlong(strList *params)
    {
    long l;

    fread(&l, sizeof(l), 1, pCurScript->UsrFile[evaluateInt(params->string)]);
    SETLONG(l);
    }

void sfReadulong(strList *params)
    {
    ulong ul;

    fread(&ul, sizeof(ul), 1, pCurScript->UsrFile[evaluateInt(params->string)]);
    SETLONG(ul);
    }

void sfReadbool(strList *params)
    {
    int b;

    fread(&b, sizeof(b), 1, pCurScript->UsrFile[evaluateInt(params->string)]);
    SETBOOL(b);
    }

void sfReadChar(strList *params)
    {
    char C;

    fread(&C, sizeof(char), 1,
            pCurScript->UsrFile[evaluateInt(params->string)]);

    SETINT(C);
    }

void sfWriteChar(strList *params)
    {
    char C = (char) evaluateInt(params->next->string);

    fwrite(&C, sizeof(char), 1,
            pCurScript->UsrFile[evaluateInt(params->string)]);
    }

void sfWritestr(strList *params)
    {
    const char *str = evaluateString(params->next->string);
    int fn = evaluateInt(params->string);

    fwrite(str, sizeof(char), strlen(str), pCurScript->UsrFile[fn]);

    switch (evaluateInt(params->next->next->string))
        {
        case 1:
            {
            char c = 0;
            fwrite(&c, sizeof(c), 1, pCurScript->UsrFile[fn]);
            break;
            }

        case 2:
            {
            char c = 13;    // cr
            fwrite(&c, sizeof(c), 1, pCurScript->UsrFile[fn]);
            c = 10;         // lf
            fwrite(&c, sizeof(c), 1, pCurScript->UsrFile[fn]);
            break;
            }
        }
    }

void sfWriteint(strList *params)
    {
    int i = evaluateInt(params->next->string);

    fwrite(&i, sizeof(i), 1, pCurScript->UsrFile[evaluateInt(params->string)]);
    }

void sfWriteuint(strList *params)
    {
    uint ui = evaluateUint(params->next->string);

    fwrite(&ui, sizeof(ui), 1, pCurScript->UsrFile[evaluateInt(params->string)]);
    }

void sfWritelong(strList *params)
    {
    long l = evaluateLong(params->next->string);

    fwrite(&l, sizeof(l), 1, pCurScript->UsrFile[evaluateInt(params->string)]);
    }

void sfWriteulong(strList *params)
    {
    ulong ul = evaluateUlong(params->next->string);

    fwrite(&ul, sizeof(ul), 1, pCurScript->UsrFile[evaluateInt(params->string)]);
    }

void sfWritebool(strList *params)
    {
    int b = evaluateInt(params->next->string);

    fwrite(&b, sizeof(b), 1, pCurScript->UsrFile[evaluateInt(params->string)]);
    }

void sfSeekfile(strList *params)
    {
    fseek(pCurScript->UsrFile[evaluateInt(params->string)],
            evaluateLong(params->next->string),
            evaluateInt(params->next->next->string));
    }

void sfTellfile(strList *params)
    {
    SETULONG(ftell(pCurScript->UsrFile[evaluateInt(params->string)]));
    }

void sfReadAplOnExit(strList *)
    {
    pCurScript->ReadAplOnExit = TRUE;
    }

void sfWriteapl(strList *params)
    {
    FILE *fl;
    char path[128];

    sprintf(path, sbs, cfg.aplpath, inputApl);

    if ((fl = fopen(path, FO_A)) != NULL)
        {
        pCurScript->ReadAplOnExit = TRUE;

        fprintf(fl, getmsg(1496), evaluateInt(params->string),
                evaluateString(params->next->string), bn);
        fclose(fl);
        SETBOOL(TRUE);
        }
    else
        {
        SETBOOL(FALSE);
        }
    }

void sfFileexists(strList *params)
    {
    if (params->next)
        {
        changedir(evaluateString(params->next->string));
        }
    else
        {
        if (!strchr(evaluateString(params->string), '\\') &&
                !strchr(evaluateString(params->string), ':'))
            {
            changedir(cfg.ScriptPath);
            }
        }

    SETBOOL(filexists(evaluateString(params->string)));
    }

void sfChangedir(strList *params)
    {
    if (params)
        {
        changedir(evaluateString(params->string));
        }
    else
        {
        changedir(cfg.homepath);
        }
    }

void sfGetDirectory(strList *params)
    {
    RequiresScriptTW();

    if (*evaluateString(params->string))
        {
        changedir(evaluateString(params->string));
        }

    delete [] pCurScript->Files;
    pCurScript->Files = ScriptTW()filldirectory(
            evaluateString(params->next->string),
            (SortE) evaluateInt(params->next->next->string), OldAndNew,
            evaluateBool(params->next->next->next->string));

    SETBOOL(!!pCurScript->Files);
    }

void sfCountDirectory(strList *)
    {
    if (pCurScript->Files)
        {
        int entries;
        for (entries = 0; *pCurScript->Files[entries].Name; ++entries);
        SETLONG(entries);
        }
    else
        {
        SETLONG(0);
        }
    }

void sfDumpDirectory(strList *)
    {
    delete [] pCurScript->Files;
    pCurScript->Files = NULL;
    }

void sfDirectoryGetTime(strList *params)
    {
    if (pCurScript->Files)
        {
        SETLONG(pCurScript->Files[evaluateInt(params->string) - 1].DateTime);
        }
    }

void sfDirectoryGetSize(strList *params)
    {
    if (pCurScript->Files)
        {
        SETLONG(pCurScript->Files[evaluateInt(params->string) - 1].Length);
        }
    }

void sfDirectoryGetName(strList *params)
    {
    if (pCurScript->Files)
        {
        SETSTR(pCurScript->Files[evaluateInt(params->string) - 1].Name);
        }
    }

void sfIsFilenameLegal(strList *params)
    {
    Bool AllowSpace = params->next ?
            evaluateBool(params->next->string) : FALSE;

    SETBOOL(IsFilenameLegal(evaluateString(params->string), AllowSpace));
    }

void sfRenamefile(strList *params)
    {
    char temp[128];
    CopyStringToBuffer(temp, evaluateString(params->next->string));

    SETBOOL(rename(evaluateString(params->string), temp) != -1);
    }
