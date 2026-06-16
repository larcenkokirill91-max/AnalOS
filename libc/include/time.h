#pragma once
#ifndef TIME_H
#define TIME_H
int rtc_is_updating(void);
void get_rtc_time(int* hours, int* minutes, int* seconds);
#endif
