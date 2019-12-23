#include <nds.h>
#include "swi.h"
#include "rtc.h"

// Delay (in swiDelay units) for each bit transfer
#define RTC_DELAY 48

// Pin defines on RTC_CR
#define CS_0    (1<<6)
#define CS_1    ((1<<6) | (1<<2))
#define SCK_0   (1<<5)
#define SCK_1   ((1<<5) | (1<<1))
#define SIO_0   (1<<4)
#define SIO_1   ((1<<4) | (1<<0))
#define SIO_out (1<<4)
#define SIO_in  (1)

void rtc_doTransfer(uint8 * command, uint32 commandLength, uint8 * result, uint32 resultLength)
{
	uint32 bit;
	uint8 data;

	// Raise CS
	RTC_CR8 = CS_0 | SCK_1 | SIO_1;
	swi_delay(RTC_DELAY);
	RTC_CR8 = CS_1 | SCK_1 | SIO_1;
	swi_delay(RTC_DELAY);

	// Write command byte (high bit first)
		data = *command++;

		for (bit = 0; bit < 8; bit++) {
			RTC_CR8 = CS_1 | SCK_0 | SIO_out | (data>>7);
			swi_delay(RTC_DELAY);

			RTC_CR8 = CS_1 | SCK_1 | SIO_out | (data>>7);
			swi_delay(RTC_DELAY);

			data = data << 1;
		}
	// Write parameter bytes (low bit first)
	for ( ; commandLength > 1; commandLength--) {
		data = *command++;

		for (bit = 0; bit < 8; bit++) {
			RTC_CR8 = CS_1 | SCK_0 | SIO_out | (data & 1);
			swi_delay(RTC_DELAY);

			RTC_CR8 = CS_1 | SCK_1 | SIO_out | (data & 1);
			swi_delay(RTC_DELAY);

			data = data >> 1;
		}
	}

	// Read result bytes (low bit first)
	for ( ; resultLength > 0; resultLength--) {
		data = 0;

		for (bit = 0; bit < 8; bit++) {
			RTC_CR8 = CS_1 | SCK_0;
			swi_delay(RTC_DELAY);

			RTC_CR8 = CS_1 | SCK_1;
			swi_delay(RTC_DELAY);

			if (RTC_CR8 & SIO_in) data |= (1 << bit);
		}
		*result++ = data;
	}

	// Finish up by dropping CS low
	RTC_CR8 = CS_0 | SCK_1;
	swi_delay(RTC_DELAY);
}