#include <stdio.h>
#include <stdlib.h>

#define	BITS	10
#define	SIZE	1024

// available: https://stackoverflow.com/questions/746171/most-efficient-algorithm-for-bit-reversal-from-msb-lsb-to-lsb-msb-in-c
unsigned int reverse(register unigned int x)
{
	x = (((x & 0xaaaaaaaa) >> 1) | ((x & 0x55555555) << 1));
	x = (((x & 0xcccccccc) >> 2) | ((x & 0x33333333) << 2));
	x = (((x & 0xf0f0f0f0) >> 4) | ((x & 0x0f0f0f0f) << 4));
	x = (((x & 0xff00ff00) >> 8) | ((x & 0x00ff00ff) << 8));
	return ((x >> 16) | (x << 16));
}

int main(void)
{
	unsigned int arr[SIZE];
	for (unsigned int i = 0; i < SIZE; ++i)
	{
		arr[i] = i;
	}
	for (unsigned int i = 0; i < SIZE; ++i)
	{
		printf("%ud\n",arr[i]);
	}
	return 0;
}
