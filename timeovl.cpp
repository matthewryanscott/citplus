// --------------------------------------------------------------------------
// Citadel: TimeOvl.CPP
//
// This file contains functions relating to the time and date.

#include "ctdl.h"
#pragma hdrstop


#include "extmsg.h"

// --------------------------------------------------------------------------
// Contents
//
// changedate()     Menu-level change date routine


// --------------------------------------------------------------------------
// changedate(): Menu-level change date routine.

void TERMWINDOWMEMBER changeDate(void)
    {
#ifndef WINCIT
    label dtstr;

    SetDoWhat(SYSDATE);

    strftime(dtstr, LABELSIZE, getmsg(435), 0l);
    CRmPrintf(getsysmsg(80), dtstr);

    strftime(dtstr, LABELSIZE, getmsg(434), 0l);
    CRmPrintfCR(getsysmsg(81), dtstr);

    if (getYesNo(getsysmsg(82), 0))
        {
        union REGS inregs, outregs;
        int first;
        label usrIn;
        datestruct ds;
        timestruct ts;

        first = TRUE;
        do
            {
            if (!HaveConnectionToUser())
                {
                return;
                }

            if (!first)
                {
                mPrintfCR(getmsg(156));
                }

            first = FALSE;

            if (!GetStringWithBlurb(getmsg(266), usrIn, LABELSIZE, ns,
                    B_DATESET))
                {
                return;
                }

            gdate(usrIn, &ds);
            } while (!ds.Date);

        first = TRUE;
        do
            {
            if (!HaveConnectionToUser())
                {
                return;
                }

            if (!first)
                {
                mPrintfCR(getmsg(121));
                }

            first = FALSE;

            if (!GetStringWithBlurb(getmsg(230), usrIn, LABELSIZE, ns,
                    B_TIMESET))
                {
                return;
                }

            gtime(usrIn, &ts);
            } while (ts.Hour < 0);

        trap(T_SYSOP, getmsg(235));

        inregs.x.cx = ds.Year + 1900;
        inregs.h.dh = ds.Month + 1;
        inregs.h.dl = ds.Date;
        inregs.h.ah = 0x2b;
        intdos(&inregs, &outregs);

        inregs.h.ch = ts.Hour;
        inregs.h.cl = ts.Minute;
        inregs.h.dh = ts.Second;
        inregs.h.ah = 0x2d;
        intdos(&inregs, &outregs);

        time(&LastActiveTime);

        strftime(dtstr, LABELSIZE, getmsg(435), 0l);
        CRmPrintf(getsysmsg(80), dtstr);

        strftime(dtstr, LABELSIZE, getmsg(434), 0l);
        CRmPrintfCR(getsysmsg(81), dtstr);

        trap(T_SYSOP, getmsg(234));
        }
#endif
    }

char *net69_gateway_time(time_t t, char *datestring)
    {
    struct time d_time;
    struct date d_date;
    char month[4];

    unixtodos(t, &d_date, &d_time);

    strcpy(month, monthTab[d_date.da_mon - 1]);

    sprintf(datestring, getmsg(233), d_date.da_year % 100,
            month, d_date.da_day, d_time.ti_hour, d_time.ti_min,
            d_time.ti_sec);

    return (datestring);
    }

time_t net69_time(const char *datestring)
    {
    struct time d_time;
    struct date d_date;
    int i;
    char year[3];
    char month[4];
    char day[3];
    char hour[3];
    char min[3];
    char sec[3];

    time_t t;

    year[0] = datestring[0];
    year[1] = datestring[1];
    year[2] = 0;

    month[0] = datestring[2];
    month[1] = datestring[3];
    month[2] = datestring[4];
    month[3] = 0;

    day[0] = datestring[5];
    day[1] = datestring[6];
    day[2] = 0;

    hour[0] = datestring[8];
    hour[1] = datestring[9];
    hour[2] = 0;

    min[0] = datestring[11];
    min[1] = datestring[12];
    min[2] = 0;

    sec[0] = datestring[14];
    sec[1] = datestring[15];
    sec[2] = 0;

    for (i = 0; i < 12; i++)
        {
        if (SameString(month, monthTab[i]))
            {
            sprintf(month, pctd, i + 1);    // i hate dostounix
            break;
            }
        }

    d_date.da_day   = (char) atoi(day);
    d_date.da_mon   = (char) atoi(month);
    d_date.da_year  = 1900 + atoi(year);    // i hate dostounix
    if (d_date.da_year < 1939)
        {
        d_date.da_year += 100;
        }

    d_time.ti_hour  = (char) atoi(hour);
    d_time.ti_min   = (char) atoi(min);
    d_time.ti_sec   = (char) atoi(sec);

    t = dostounix(&d_date, &d_time);

    return (t);
    }

void gtime(char *line, timestruct *filltime)
    {
    label collector;
    int i, j = 0, offset;

    for (i = 0; isspace(line[i]); i++);

    if (!line[i])
        {
        long tnow;
        struct tm *tmnow;

        time(&tnow);
        tmnow = localtime(&tnow);

        filltime->Hour = tmnow->tm_hour;
        filltime->Minute = tmnow->tm_min;
        filltime->Second = tmnow->tm_sec;
        return;
        }

    if (line[i] == '-')
        {
        offset = -1;
        i++;
        }
    else if (line[i] == '+')
        {
        offset = 1;
        i++;
        }
    else
        {
        offset = 0;
        }

    while (isdigit(line[i]) && j < LABELSIZE)
        {
        collector[j++] = line[i++];
        }
    collector[j] = 0;

    filltime->Hour = atoi(collector);

    if (line[i] == ':')
        {
        j = 0;
        i++;

        while (isdigit(line[i]) && j < LABELSIZE)
            {
            collector[j++] = line[i++];
            }
        collector[j] = 0;

        filltime->Minute = atoi(collector);

        if (line[i] == ':')
            {
            j = 0;
            i++;

            while (isdigit(line[i]) && j < LABELSIZE)
                {
                collector[j++] = line[i++];
                }
            collector[j] = 0;

            filltime->Second = atoi(collector);
            }
        else
            {
            filltime->Second = 0;
            }
        }
    else
        {
        filltime->Minute = 0;
        filltime->Second = 0;
        }

    while (isspace(line[i]))
        {
        i++;
        }

    j = i;
    while (isalpha(line[j]))
        {
        j++;
        }
    line[j] = 0;

    if (line[i])
        {
        if (toupper(line[i]) == toupper(amPtr[0]))
            {
            // nothing
            }
        else if (toupper(line[i]) == toupper(pmPtr[0]))
            {
            filltime->Hour += 12;
            }
        else
            {
            filltime->Hour = -1;
            filltime->Minute = -1;
            filltime->Second = -1;
            return;
            }
        }

    if (filltime->Hour > 24 || filltime->Hour < 0 ||
            filltime->Minute > 59 || filltime->Minute < 0 ||
            filltime->Second > 59 || filltime->Second < 0)
        {
        filltime->Hour = -1;
        filltime->Minute = -1;
        filltime->Second = -1;
        return;
        }

    if (offset)
        {
        long tnow;
        struct tm *tmnow;

        time(&tnow);

        tnow += offset * (filltime->Hour * 3600l + filltime->Minute * 60l +
                    filltime->Second);

        tmnow = localtime(&tnow);

        filltime->Hour = tmnow->tm_hour;
        filltime->Minute = tmnow->tm_min;
        filltime->Second = tmnow->tm_sec;
        }
    }


// --------------------------------------------------------------------------
//  gdate.c
//
//  Assumptions:
//      Default to past (month, date)
//      Don't use three digits in a row (1 or 2 or 4 okay)
//  Basic algorithm:
//      If returned date==0, error parsing
//      Blank out (space-fill) stuff after finding
//
//      Search for +/- till first digit, if found set offset flag
//      Search for TLA month, if found and offset, error
//      Search for 'xx or 19xx or 20xx year
//      If no year yet, search for #>31 for year
//      If year and offset, error
//      If 1 number, date
//      Fill out other fields in month, date, year order if zeroed

static int eval2digits(const char *ptr);
static int eval1or2digits(const char *ptr);
static void fillCurrentDate(int offset, datestruct *filldate);

void gdate(char *line, datestruct *filldate)
    // both args mauled
    {
    int offset = 0;     // leading +/-
    int i;
    char lcMonthTab[4];
    char *ptr, *ptr2, *ptr3;

    filldate->Year = 0;
    filldate->Month = NUM_MONTHS;
    filldate->Date = 0;

    strlwr(line);                   // easier searching
    line[strlen(line) + 1] = '\0';  // two nuls at end, so we can ptr += 2

    // Remove leading spaces
    while (isspace(*line))
        {
        line++;
        }

    // Check if offset
    for (ptr = line; *ptr && !isdigit(*ptr); ptr++)
        {
        if ('+' == *ptr)
            {
            offset = 1;
            break;
            }
        else
            if ('-' == *ptr)
            {
            offset = -1;
            break;
            }
        }


    // Search for month name
    // Good thing that no TLA month is contained within another
    for (i = 0; i < NUM_MONTHS; i++)
        {
        strcpy(lcMonthTab, monthTab[i]);
        strlwr(lcMonthTab);
        ptr = strstr(line, lcMonthTab);

        if (NULL != ptr)
            {
            if (offset)                 // no offsets with TLA months
                {
                return;
                }

            filldate->Month = (MonthsE) i;

            while (*ptr && isalpha(*ptr))
                {
                *ptr++ = ' ';
                }

            break;
            }
        }

    // Make sure that no additional letters are in the string
    for (ptr = line; *ptr; ptr++)
        {
        if (isalpha(*ptr))
            {
            return;
            }
        }

    // Search for century (' or 19 or 20)
    // If digits directly follow, it should be a year

    if (NULL != (ptr = (strchr(line, '\''))))
        {
        if (-1 == (filldate->Year = eval2digits(ptr + 1)))
            return;
        }
    if (!filldate->Year && NULL != (ptr = (strstr(line, getmsg(71)))))
        {
        if (-1 == (filldate->Year = eval2digits(ptr + 2)))
            return;
        }
    // Skip 20xx if got 19xx
    if (!filldate->Year && NULL != (ptr = (strstr(line, getmsg(14)))))
        {
        if (-1 == (filldate->Year = eval2digits(ptr + 2)))
            return;
        }

    // Now fill with spaces if found
    if (filldate->Year)     // filled
        {
        for (i = 0; i < 4; i++)
            {
            *ptr++ = ' ';
            }
        }
    else    // look for year > 31 or == 0
        {
        for (ptr = line; *ptr; ptr++)
            {
            if (!isdigit(*ptr))
                {
                continue;
                }
            i = eval2digits(ptr);   // must be two digits for >31
            if (i > 31 || !i)
                {
                filldate->Year = i;
                *ptr++ = ' ';
                *ptr = ' ';
                break;
                }
            }
        }

    if (filldate->Year && offset)   // no years with offsets
        {
        return;
        }


    // Let's divy up the rest of the string into numbers, see what we have

    for (ptr = ptr2 = ptr3 = line; *ptr; ptr++)
        {
        if (isdigit(*ptr))
            {
            break;
            }
        }
    if (*ptr)
        {
        for (ptr2 = ptr3 = ptr + 2; *ptr2; ptr2++)
            {
            if (isdigit(*ptr2))
                {
                break;
                }
            }
        if (*ptr2)
            {
            for (ptr3 = ptr2 + 2; *ptr3; ptr3++)
                {
                if (isdigit(*ptr3))
                    {
                    break;
                    }
                }
            }
        }

    if (*ptr)   // Is there a first number?
        {
        if (!isdigit(*ptr2))        // Only one number?
            {
            filldate->Date = eval1or2digits(ptr);
            }
        else if (filldate->Month && !filldate->Year && ptr == line)
            {
            filldate->Year = eval1or2digits(ptr);
            }
        else if (filldate->Month == NUM_MONTHS) // It's a month
            {
            filldate->Month = (MonthsE) (eval1or2digits(ptr) - 1);
            }
        else                        // Nope, it really is a date!
            {
            filldate->Date = eval1or2digits(ptr);
            }

        if (*ptr2)  // Is there a second number? Month full as it gets
            {
            if (!filldate->Date)
                {
                filldate->Date = eval1or2digits(ptr2);
                }
            else if (!filldate->Year)
                {
                filldate->Year = eval1or2digits(ptr2);
                }
            else
                {
                filldate->Date = 0; // parse error
                return;
                }

            if (*ptr3)  // Third, got to be year
                {
                if (!filldate->Year)
                    {
                    filldate->Year = eval1or2digits(ptr3);
                    }
                else
                    {
                    filldate->Date = 0; // parse error
                    return;
                    }
                }
            }
        }

    if (filldate->Year && offset)   // no years with offsets
        {
        filldate->Date = 0;
        return;
        }

    fillCurrentDate(offset, filldate);
    return;
}


static int eval2digits(const char *ptr)
    {
    if (isdigit(ptr[0]))
        {
        if (isdigit(ptr[1]))
            {
            const int i = 10 * (ptr[0] - '0') + ptr[1] - '0';
            return (!i ? 100 : i);      // If 0, return 100
            }
        else
            {
            return (-1);
            }
        }
    else
        {
        return (0);
        }
    }

static int eval1or2digits(const char *ptr)
    {
    int i = *ptr++ - '0';

    if (isdigit(*ptr))
        {
        i = 10 * i + *ptr - '0';
        }

    return (!i ? 100 : i);      // If 0, return 100
    }

static void fillCurrentDate(int offset, datestruct *filldate)
    {
    struct tm *current;
    time_t long_time;

    // Get current date, would be nice if these lines could be combined
    time(&long_time);
    long_time += filldate->Date * offset * SECSINDAY; // secs/day
    current = localtime(&long_time);
//  Bool addmonth = (filldate->Date <= current->tm_mday);

    if (!filldate->Year)
        {
        filldate->Year = current->tm_year;
        }
    else if (filldate->Year < 39)   // Keep "100" as-is, add if 1-38
        {
        filldate->Year += 100;
        }

    if (offset)
        {
        filldate->Month = (MonthsE) (current->tm_mon +
                ((filldate->Month == NUM_MONTHS) ? 0 :
                filldate->Month * offset));

        filldate->Date = current->tm_mday;  // already got offset

        while (filldate->Month < M_JAN)     // back a year?
            {
            filldate->Month = (MonthsE) (filldate->Month + 12);
            filldate->Year--;
            }

        while (filldate->Month > M_DEC)     // forward a year?
            {
            filldate->Month = (MonthsE) (filldate->Month - 12);
            filldate->Year++;
            }
        }
    else
        {
        if (!filldate->Date)
            {
            if (filldate->Month != NUM_MONTHS)
                {
                filldate->Date = 1;
                }
            else
                {
                filldate->Date = current->tm_mday;
                }
            }

        if (filldate->Month == NUM_MONTHS)
            {
            filldate->Month = (MonthsE) current->tm_mon; //+ addmonth;
            }
        }

    switch (filldate->Month)    // Correct for date overflow
        {
        case M_FEB:
            {
            if (filldate->Date >= 30)
                {
                if (offset)
                    {
                    filldate->Date = 28;
                    }
                else
                    {
                    filldate->Month = (MonthsE) (filldate->Month - 1);
                    // += 1 - 2 * addmonth;
                    }
                }

            break;
            }

        case M_APR:
        case M_JUN:
        case M_SEP:
        case M_NOV:
            {
            if (filldate->Date >= 31)
                {
                if (offset)
                    {
                    filldate->Date = 30;
                    }
                else
                    {
                    filldate->Month = (MonthsE) (filldate->Month - 1);
                    // += 1 - 2 * addmonth;
                    }
                }

            break;
            }
        }

    if (filldate->Date > 31 ||
        filldate->Month < M_JAN ||
        filldate->Month > M_DEC)    // lots of huge numbers, anyone?
        {
        filldate->Date = 0;
        }
    }
