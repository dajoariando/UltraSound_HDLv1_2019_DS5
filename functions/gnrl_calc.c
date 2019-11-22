#include <math.h>
#include <stdio.h>

unsigned int us_to_clk_cycles (double time_us, double freq_MHz) {
	unsigned int cycles;

	cycles = (unsigned int)(lround(time_us*freq_MHz));
	printf("cycles : %d\n",cycles);

	return cycles;
}
