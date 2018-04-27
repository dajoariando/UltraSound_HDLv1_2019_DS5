#include "cpmg_functions.h"

void cpmg_param_calculator_ltc2314 (
	unsigned int * output,		// cpmg parameter output
	double cpmg_freq,			// cpmg operating frequency
	double adc_sampling_freq,	// adc sampling frequency (for LTC2134 it is 4.5 Msps)
	double adc_pll_freq,		// adc pll frequency for LTC2134 (max at 175 MHz)
	double shift_sampling_us,	// shift the 180 deg data capture relative to the middle of the 180 delay span. This is to compensate shifting because of signal path delay / other factors. This parameter could be negative as well
	double pulse1_us,			// the length of cpmg 90 deg pulse
	double pulse2_us,			// the length of cpmg 180 deg pulse
	double echotime_us,			// the length between one echo to the other (equal to pulse2_us + delay2_us)
	unsigned int total_sample	// the total adc samples captured in one echo
){
	
	//output
	double pulse1_int;
	double delay1_int;
	double pulse2_int;
	double delay2_int;
	double init_adc_delay_int;

	pulse1_int = pulse1_us * cpmg_freq; // the number of 90 deg pulse in the multiplication of cpmg pulse period (discrete value, no continuous number supported)
	pulse2_int = pulse2_us * cpmg_freq;	// the number of 180 deg pulse in the multiplication of cpmg pulse period (discrete value, no continuous number supported)
	delay1_int = (echotime_us/2)*cpmg_freq - pulse1_int;	// the number of delay after 90 deg pulse
	delay2_int = echotime_us*cpmg_freq - pulse2_int;		// the number of delay after 180 deg pulse

	init_adc_delay_int = ((echotime_us/2) - ((((double)total_sample)/2)/adc_sampling_freq) - pulse2_us + shift_sampling_us)*adc_pll_freq;	// the amount of delay needed to make data capture exactly in the middle of the delay span after 180 deg pulse. Shift_sampling_us is therefore designed to shift the data capture by an exact amount of time 

	// put all the numbers into the output
	*(output+PULSE1_OFFST) = (unsigned int)pulse1_int;
	*(output+DELAY1_OFFST) = (unsigned int)delay1_int;
	*(output+PULSE2_OFFST) = (unsigned int)pulse2_int;
	*(output+DELAY2_OFFST) = (unsigned int)delay2_int;
	*(output+INIT_DELAY_ADC_OFFST) = (unsigned int)init_adc_delay_int;
}

void cpmg_param_calculator_ltc1746 (
	unsigned int * output,		// cpmg parameter output
	double nmr_fsm_clkfreq,		// nmr fsm operating frequency (in MHz)
	double adc_sampling_freq,	// adc sampling frequency (for LTC1746 it is 25 Msps)
	double shift_sampling_us,	// shift the 180 deg data capture relative to the middle of the 180 delay span. This is to compensate shifting because of signal path delay / other factors. This parameter could be negative as well
	double pulse1_us,			// the length of cpmg 90 deg pulse
	double pulse2_us,			// the length of cpmg 180 deg pulse
	double echotime_us,			// the length between one echo to the other (equal to pulse2_us + delay2_us)
	unsigned int total_sample	// the total adc samples captured in one echo
){
	
	//output
	unsigned int pulse1_int;
	unsigned int delay1_int;
	unsigned int pulse2_int;
	unsigned int delay2_int;
	unsigned int init_adc_delay_int;

	//residue for round test
	double pulse1_res;
	double pulse2_res;
	double delay1_res;
	double delay2_res;

	pulse1_int = (unsigned int)(pulse1_us * nmr_fsm_clkfreq); // the number of 90 deg pulse in the multiplication of cpmg pulse period (discrete value, no continuous number supported)
	pulse2_int = (unsigned int)(pulse2_us * nmr_fsm_clkfreq);	// the number of 180 deg pulse in the multiplication of cpmg pulse period (discrete value, no continuous number supported)
	delay1_int = (unsigned int)((echotime_us/2)*nmr_fsm_clkfreq) - pulse1_int;	// the number of delay after 90 deg pulse
	delay2_int = (unsigned int)(echotime_us*nmr_fsm_clkfreq) - pulse2_int;		// the number of delay after 180 deg pulse

	// this init_adc_delay is not really precise due to floor function conversion to unsigned int! use with caution. It will introduce phase shift
	init_adc_delay_int = (unsigned int) ( ((echotime_us/2) - ((((double)total_sample)/2)/adc_sampling_freq) - pulse2_us + shift_sampling_us)*adc_sampling_freq );	// the amount of delay needed to make data capture exactly in the middle of the delay span after 180 deg pulse. Shift_sampling_us is therefore designed to shift the data capture by an exact amount of time

	// it is really important to test the integer value so the double value is not floored by integer conversion e.g when the number 4.999.
	// this floor will introduce a problem, because most of the time the pulse2_int + delay2_int is not equal to the desired value of echotime_us
	// for some reasons, the difference between this, will make acquisition error.
	// probably due to 20ns error, which is half of the 40ns ADC sampling rate, shifted data by 180 deg,
	// causing data cancellation between each other for every echoes.
	pulse1_res = ((double)pulse1_int / nmr_fsm_clkfreq) - pulse1_us;
	if (pulse1_res > 0.5) {
		pulse1_int--;
	}
	if (pulse1_res < -0.5) {
		pulse1_int++;
	}

	pulse2_res = ((double)pulse2_int / nmr_fsm_clkfreq) - pulse2_us;
	if (pulse2_res > 0.5) {
		pulse2_int--;
	}
	if (pulse2_res < -0.5) {
		pulse2_int++;
	}

	delay1_res = ((double)(delay1_int+pulse1_int) / nmr_fsm_clkfreq) - (echotime_us/2);
	if (delay1_res > 0.5) {
		delay1_int--;
	}
	if (delay1_res < -0.5) {
		delay1_int++;
	}

	delay2_res = ((double)(delay2_int+pulse2_int) / nmr_fsm_clkfreq) - echotime_us;
	if (delay2_res > 0.5) {
		delay2_int--;
	}
	if (delay2_res < -0.5) {
		delay2_int++;
	}



	// put all the numbers into the output
	*(output+PULSE1_OFFST) = (unsigned int)pulse1_int;
	*(output+DELAY1_OFFST) = (unsigned int)delay1_int;
	*(output+PULSE2_OFFST) = (unsigned int)pulse2_int;
	*(output+DELAY2_OFFST) = (unsigned int)delay2_int;
	*(output+INIT_DELAY_ADC_OFFST) = (unsigned int)init_adc_delay_int;
}
