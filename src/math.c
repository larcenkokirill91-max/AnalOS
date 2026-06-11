#include "math.h"
int floor_div(int a, int b) {
	int q = a / b;
	int r = a % b;
	if (r != 0 && (a < 0) != (b < 0))
		q--;
	return q;}
int tenth_digit(int a, int b) {
	int num = a;
	int den = b;
	if (den < 0) {
		num = -num;
		den = -den;
	}
	if (num < 0)
		num = -num;
	int scaled = (num * 10) / den;
	return scaled % 10;}