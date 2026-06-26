#pragma once
#ifndef TIME_H
#define TIME_H
int rtc_is_updating(void);
void get_rtc_time(int* years, int* mounth, int* days, int* hours, int* minutes, int* seconds);
void timer_wait(int ticks);
#endif