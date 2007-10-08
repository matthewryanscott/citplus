#ifndef WINCIT
#include <dos.h>
#include <stdlib.h>


#include "timer.h"

/*
	Our link to TIMERA.ASM: OurTimerHandler is a new ISR for INT 08h, and
	OurTimerMSW is the MSW of our timer, which is handled by OurTimerHandler.
*/

extern "C" void interrupt OurTimerHandler(...);
extern unsigned int cdecl OurTimerMSW;


/*
	The timer state before we get to it. WasSquare stores if the timer was
	in square wave mode. SystemTimerHandler is the address of the old
	INT 08h ISR. This is used by OurTimerHandler in TIMERA.ASM, so it is
	not declared static.
*/
void interrupt (* cdecl SystemTimerHandler)(...);
static int WasSquare;


// These next three functions (including dummy()) are from Borland.
// And copyrighted by Borland.  And this is less than legal.
static void near dummy (void) {}

/*--------------------------------------------------------------------------*

Name            readtimer - read the complemented value of timer 0

Usage           unsigned readtimer (void);

Prototype in    local

Description     Obtain the complement of the value in timer 0.  The
                complement is used so that the timer will appear to
                count up rather than down.  The value returned will
                range from 0 to 0xffff.

Return value    The complement of the value in timer 0.

*---------------------------------------------------------------------------*/

static unsigned int ReadTimerLSW(void)
	{
	asm pushf				/* Save interrupt flag      */
	asm cli                 /* Disable interrupts       */
	asm mov  al,0h          /* Latch timer 0            */
	asm out  43h,al
	dummy();                /* Waste some time			*/
	asm in   al,40h         /* Counter --> bx           */
	asm mov  bl,al          /* LSB in BL                */
	dummy();                /* Waste some time			*/
	asm in   al,40h
	asm mov  bh,al          /* MSB in BH                */
	asm not  bx             /* Need ascending counter   */
	asm popf                /* Restore interrupt flag	*/
	return( _BX );
	}

static int IsTimerSquareWave(void)
	{
	for (int i = 0; i < 100; i++)
		{
		if ((ReadTimerLSW() & 1) == 0)     /* readtimer() returns complement */
			{
			return (0);
			}
		}

	return (1);
	}

/*--------------------------------------------------------------------------*

Name            timer_init - initialize multiplier for delay function

Usage           void timer_init (void);

Prototype in    local

Description     Determine the multiplier required to convert milliseconds
                to an equivalent interval timer value.  Interval timer 0
                is normally programmed in mode 3 (square wave), where
                the timer is decremented by two every 840 nanoseconds;
                in this case the multiplier is 2386.  However, some
                programs and device drivers reprogram the timer in mode 2,
                where the timer is decremented by one every 840 ns; in this
                case the multiplier is halved, i.e. 1193.

                When the timer is in mode 3, it will never have an odd value.
                In mode 2, the timer can have both odd and even values.
                Therefore, if we read the timer 100 times and never
                see an odd value, it's a pretty safe assumption that
                it's in mode 3.  This is the method used in timer_init.

Return value    None

*---------------------------------------------------------------------------*/

static long GetOurTimerValue(void)
	{
	return ((((long) OurTimerMSW) << 16) | (long) ReadTimerLSW());
	}

#include <stdio.h>

void InitializeTimer(void)
	{
	if (!SystemTimerHandler)
		{
		// Store old state of 8253-5 Programmable Interval Timer.
		WasSquare = IsTimerSquareWave();

		// Set to mode 2.
		asm mov al, 00110100b
		asm out 43h, al
		dummy();

		// And keep it at the standard speed.
		asm mov al, 0ffh
		asm out 40h, al
		dummy();
		asm out 40h, al

		SystemTimerHandler = getvect(0x08);
		setvect(0x08, OurTimerHandler);
		}
	}

void DeinitializeTimer(void)
	{
	if (SystemTimerHandler)
		{
		setvect(0x08, SystemTimerHandler);
		SystemTimerHandler = NULL;

		if (WasSquare)
			{
			// Set back to mode 3.
			asm mov al, 00110110b
			asm out 43h, al
			dummy();

			// And keep it at the standard speed.
			asm mov al, 0ffh
			asm out 40h, al
			dummy();
			asm out 40h, al
			}
		}
	}

Timer::Timer(void)
	{
	Reset();
	}

void Timer::Reset(void)
	{
	StartValue = GetOurTimerValue();
	}

unsigned long Timer::Read(void)
	{
	return ((GetOurTimerValue() - StartValue));
	}
#endif
