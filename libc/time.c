#include <kernel.h>
int rtc_is_updating(void) {
	outb(0x70, 0x0A);
	volatile unsigned char status = inb(0x71);
	return (status & 0x80);}
void get_rtc_time(int* hours, int* minutes, int* seconds) {
	outb(0x70, 0x00); unsigned char bcd_sec = inb(0x71);
	outb(0x70, 0x02); unsigned char bcd_min = inb(0x71);
	outb(0x70, 0x04); unsigned char bcd_hrs = inb(0x71);
	*seconds = (bcd_sec & 0x0F) + ((bcd_sec >> 4) * 10);
	*minutes = (bcd_min & 0x0F) + ((bcd_min >> 4) * 10);
	*hours   = (bcd_hrs & 0x0F) + ((bcd_hrs >> 4) * 10);}
