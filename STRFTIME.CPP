// --------------------------------------------------------------------------
// Citadel: StrFTime.CPP
//
// Time/Date String Formatting stuff

#include "ctdl.h"
#pragma hdrstop

// --------------------------------------------------------------------------
// Contents
//
// strftime()       formats a custom time and date string using formats


// --------------------------------------------------------------------------
// strftime(): Formats a custom time and date string using formats.


void strftime(char *outstr, int maxsize, const char *formatstr, long tnow)
    {
#ifdef VISUALC
    // I can't figure out how to make this THREAD in visual C
    static enum MonthsE lastMonthHoliday;   // The last month we checked
#else
    static MonthsE THREAD lastMonthHoliday; // The last month we checked
#endif
    static int THREAD lastDateHoliday;      // The last date we checked
    static long THREAD curHoliday;          // Pointer to all of the data

    if (tnow == -1 && !outstr && !formatstr && !maxsize)
        {
        lastMonthHoliday = NUM_MONTHS;
        lastDateHoliday = 0;
        curHoliday = 0;
        return;
        }

    int i, k;
    char temp[80];
    struct tm *tmnow;
    holidays *thisHol;

    if (tnow == 0l)
        {
        time(&tnow);
        }

    tmnow = localtime(&tnow);

    *outstr = 0;

    for (i = 0; formatstr[i]; i++)
        {
        if (formatstr[i] != '%')
            {
            sprintf(temp, pctc, formatstr[i]);
            }
        else
            {
            i++;
            temp[0] = '\0';
            if (formatstr[i])
                {
                switch (formatstr[i])
                    {
                    case 'a':
                        {
                        // %a abbreviated weekday name
                        sprintf(temp, pcts, days[tmnow->tm_wday]);
                        break;
                        }

                    case 'A':
                        {
                        // %A full weekday name
                        sprintf(temp, pcts, fulldays[tmnow->tm_wday]);
                        break;
                        }

                    case 'b':
                        {
                        // %b abbreviated month name
                        sprintf(temp, pcts, monthTab[tmnow->tm_mon]);
                        break;
                        }

                    case 'B':
                        {
                        // %B full month name
                        sprintf(temp, pcts, fullmnts[tmnow->tm_mon]);
                        break;
                        }

                    case 'c':
                        {
                        // %c standard date and time string
                        sprintf(temp, pcts, ctime(&tnow));
                        temp[strlen(temp)-1] = '\0';
                        break;
                        }

                    case 'd':
                        {
                        // %d day-of-month as decimal (1-31)
                        sprintf(temp, pctd, tmnow->tm_mday);
                        break;
                        }

                    case 'D':
                        {
                        // %D day-of-month as decimal (01-31)
                        sprintf(temp, getmsg(47), tmnow->tm_mday);
                        break;
                        }

                    case 'H':
                        {
                        // %H hour, range (00-23)
                        sprintf(temp, getmsg(47), tmnow->tm_hour);
                        break;
                        }

                    case 'I':
                        {
                        // %I hour, range (01-12)
                        if (tmnow->tm_hour)
                            {
                            sprintf(temp, getmsg(47), tmnow->tm_hour > 12 ?
                                    tmnow->tm_hour - 12 : tmnow->tm_hour);
                            }
                        else
                            {
                            sprintf(temp, pctd, 12);
                            }
                        break;
                        }

                    case 'j':
                        {
                        // %j day-of-year as a decimal (001-366)
                        sprintf(temp, getmsg(629), tmnow->tm_yday + 1);
                        break;
                        }

                    case 'm':
                        {
                        // %m month as decimal (01-12)
                        sprintf(temp, getmsg(47), tmnow->tm_mon + 1);
                        break;
                        }

                    case 'M':
                        {
                        // %M minute as decimal (00-59)
                        sprintf(temp, getmsg(47), tmnow->tm_min);
                        break;
                        }

                    case 'p':
                        {
                        // %p locale's equivaent of AM or PM
                        sprintf(temp, pcts, tmnow->tm_hour > 11 ? pmPtr : amPtr);
                        break;
                        }

                    case 'S':
                        {
                        // %S second as decimal (00-59)
                        sprintf(temp, getmsg(47), tmnow->tm_sec);
                        break;
                        }

                    case 'U':
                        {
                        // %U week-of-year, Sunday being first day (00-52)
                        k = tmnow->tm_wday - (tmnow->tm_yday % 7);
                        if (k < 0)
                            {
                            k += 7;
                            }

                        if (k != 0)
                            {
                            k = tmnow->tm_yday - (7-k);
                            if (k < 0)
                                {
                                k = 0;
                                }
                            }
                        else
                            {
                            k = tmnow->tm_yday;
                            }
                        sprintf(temp, getmsg(47), k/7);
                        break;
                        }

                    case 'W':
                        {
                        // %W week-of-year, Monday being first day (00-52)
                        k = tmnow->tm_wday - (tmnow->tm_yday % 7);
                        if (k < 0)
                            {
                            k += 7;
                            }

                        if (k != 1)
                            {
                            if (k == 0)
                                {
                                k = 7;
                                }
                            k = tmnow->tm_yday - (8-k);
                            if (k < 0)
                                {
                                k = 0;
                                }
                            }
                        else
                            {
                            k = tmnow->tm_yday;
                            }
                        sprintf(temp, getmsg(47), k/7);
                        break;
                        }

                    case 'w':
                        {
                        // %w weekday as a decimal (0-6, sunday being 0)
                        sprintf(temp, pctd, tmnow->tm_wday);
                        break;
                        }

                    case 'x':
                        {
                        // %x standard date string
                        *temp = 0;

                        // week of the month
                        k = (int) (tmnow->tm_mday - 1) / 7 + 1;

                        if (lastMonthHoliday != (MonthsE) tmnow->tm_mon ||
                                lastDateHoliday != tmnow->tm_mday)
                            {
                            lastMonthHoliday = (MonthsE) tmnow->tm_mon;
                            lastDateHoliday = tmnow->tm_mday;

                            for (thisHol = holidayList; thisHol;
                                thisHol = (holidays *) getNextLL((void *)thisHol))
                                {
                                if (thisHol->Month == (MonthsE) tmnow->tm_mon)
                                    {
                                    if (thisHol->date)
                                        {
                                        if ((thisHol->date == tmnow->tm_mday) &&
                                                (thisHol->year == -1 ||
                                                thisHol->year ==
                                                tmnow->tm_year + 1900))
                                            {
                                            curHoliday = (long) thisHol;
                                            break;
                                            }
                                        }
                                    else
                                        {
                                        if ((thisHol->day == tmnow->tm_wday &&
                                                thisHol->week == k) &&
                                                (thisHol->year == -1 ||
                                                thisHol->year ==
                                                tmnow->tm_year + 1900))
                                            {
                                            curHoliday = (long) thisHol;
                                            break;
                                            }
                                        }
                                    }
                                }

                            if (!thisHol)
                                {
                                curHoliday = (long) NULL;
                                }
                            }

                        if (curHoliday)
                            {
                            if (((holidays *) curHoliday)->year == -1)
                                {
                                sprintf(temp, getmsg(104),
                                        ((holidays *) curHoliday)->name,
                                        tmnow->tm_year % 100);
                                }
                            else
                                {
                                CopyStringToBuffer(temp,
                                        ((holidays *) curHoliday)->name);
                                }
                            }
                        else
                            {
                            sprintf(temp, getmsg(16), tmnow->tm_year % 100,
                                    monthTab[tmnow->tm_mon],
                                    tmnow->tm_mday);
                            }

                        break;
                        }

                    case 'X':
                        {
                        // %X standard time string
                        sprintf(temp, getmsg(420), tmnow->tm_hour,
                                tmnow->tm_min, tmnow->tm_sec);
                        break;
                        }

                    case 'y':
                        {
                        // %y year in decimal without century (00-99)
                        sprintf(temp, getmsg(47), tmnow->tm_year % 100);
                        break;
                        }

                    case 'Y':
                        {
                        // %Y year including century as decimal
                        sprintf(temp, pctd, 1900 + tmnow->tm_year);
                        break;
                        }

                    case 'Z':
                        {
                        // %Z timezone name
                        tzset();
//                      sprintf(temp, pcts, tzname[!!tmnow->tm_isdst]);
                        sprintf(temp, pcts, _tzname[!!tmnow->tm_isdst]);
                        break;
                        }

                    case '%':
                        {
                        // %% the percent sign
                        strcpy(temp, getmsg(4));
                        break;
                        }

                    default:
                        {
                        temp[0] = 0;
                        break;
                        }
                    }
                }
            }

        if ((int) (strlen(temp) + strlen(outstr)) > maxsize)
            {
            break;
            }
        else
            {
            if (*temp)
                {
                strcat(outstr, temp);
                }
            }
        }
    }
