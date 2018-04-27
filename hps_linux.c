#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <time.h>

#include "alt_generalpurpose_io.h"
#include <hwlib.h>
#include <socal/alt_gpio.h>
#include <socal/hps.h>
#include <socal/socal.h>
#include "./soc_variables/hps_0.h"

#include "hps_linux.h"
#include "functions/general.h"
#include "functions/avalon_i2c.h"
#include "functions/tca9555_driver.h"
#include "functions/avalon_spi.h"
#include "functions/dac_ad5722r_driver.h"
#include "functions/general.h"
#include "functions/reconfig_functions.h"
#include "functions/pll_param_generator.h"
#include "functions/adc_functions.h"
#include "functions/cpmg_functions.h"
#include "functions/AlteraIP/altera_avalon_fifo_regs.h"
#include "functions/nmr_table.h"

extern int fd_dev_mem;

void init() {
	printf("NMR SYSTEM STARTS!\n");

	// open device memory
    fd_dev_mem = open("/dev/mem", O_RDWR | O_SYNC);
    if(fd_dev_mem  == -1) {
        printf("ERROR: could not open \"/dev/mem\".\n");
        printf("    errno = %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    // mmap hps peripherals
    hps_gpio = mmap(NULL, hps_gpio_span, PROT_READ | PROT_WRITE, MAP_SHARED, fd_dev_mem, hps_gpio_ofst);
	if (hps_gpio == MAP_FAILED) {
		printf("Error: hps_gpio mmap() failed.\n");
		printf("    errno = %s\n", strerror(errno));
		close(fd_dev_mem);
		exit(EXIT_FAILURE);
	}

	// mmap fpga peripherals
	h2f_lw_axi_master = mmap(NULL, h2f_lw_axi_master_span, PROT_READ | PROT_WRITE, MAP_SHARED, fd_dev_mem, h2f_lw_axi_master_ofst);
	if (h2f_lw_axi_master == MAP_FAILED) {
		printf("Error: h2f_lw_axi_master mmap() failed.\n");
		printf("    errno = %s\n", strerror(errno));
		close(fd_dev_mem);
		exit(EXIT_FAILURE);
	}

	h2p_ctrl_out_addr				= h2f_lw_axi_master + CTRL_OUT_BASE;
	h2p_ctrl_in_addr				= h2f_lw_axi_master + CTRL_IN_BASE;
	h2p_led_addr					= h2f_lw_axi_master + LED_BASE;
	h2p_sw_addr						= h2f_lw_axi_master + SW_BASE;
	h2p_pulse180_t1_addr			= h2f_lw_axi_master + NMR_PARAMETERS_PULSE_180DEG_T1_BASE;
	h2p_delay180_t1_addr			= h2f_lw_axi_master + NMR_PARAMETERS_DELAY_NOSIG_T1_BASE;
	h2p_pulse1_addr					= h2f_lw_axi_master + NMR_PARAMETERS_PULSE_90DEG_BASE;
	h2p_pulse2_addr					= h2f_lw_axi_master + NMR_PARAMETERS_PULSE_180DEG_BASE;
	h2p_delay1_addr					= h2f_lw_axi_master + NMR_PARAMETERS_DELAY_NOSIG_BASE;
	h2p_delay2_addr					= h2f_lw_axi_master + NMR_PARAMETERS_DELAY_SIG_BASE;
	h2p_nmr_pll_addr				= h2f_lw_axi_master + PLL_NMR_RECONFIG_BASE;
	h2p_adc_pll_addr				= h2f_lw_axi_master + PLL_ADC_RECONFIG_BASE;
	h2p_echo_per_scan_addr			= h2f_lw_axi_master + NMR_PARAMETERS_ECHOES_PER_SCAN_BASE;
	h2p_adc_fifo_addr				= h2f_lw_axi_master + ADC_FIFO_MEM_OUT_BASE;
	h2p_adc_fifo_status_addr		= h2f_lw_axi_master + ADC_FIFO_MEM_OUT_CSR_BASE;
	h2p_adc_samples_per_echo_addr	= h2f_lw_axi_master + NMR_PARAMETERS_SAMPLES_PER_ECHO_BASE;
	h2p_init_adc_delay_addr			= h2f_lw_axi_master + NMR_PARAMETERS_INIT_DELAY_BASE;



	// initialize control lines to default value
	alt_write_word( (h2p_ctrl_out_addr) , ctrl_out );

	// set reconfig configuration for both of pll's
	Reconfig_Mode(h2p_nmr_pll_addr,1); // polling mode for main pll
	Reconfig_Mode(h2p_adc_pll_addr,1); // polling mode for main pll

	// skip T1 180-deg by putting its registers to 0
	// MAKE SURE TO RESET THESE VARIABLES TO 0 WHEN T1_MEASUREMENT IS NOT INTENDED
	alt_write_word( h2p_pulse180_t1_addr , 0 ); // required to skip 180-deg pulse
	alt_write_word( h2p_delay180_t1_addr , 0 ); // not required, but do it anyway

	// reset the system
	ctrl_out |= nmr_controller_reset;
	alt_write_word( (h2p_ctrl_out_addr) , ctrl_out );
	usleep(10);
	ctrl_out &= ~(nmr_controller_reset);
	usleep(10);

}

void leave() {
    close(fd_dev_mem);

    // munmap hps peripherals
    if (munmap(hps_gpio, hps_gpio_span) != 0) {
            printf("Error: hps_gpio munmap() failed\n");
            printf("    errno = %s\n", strerror(errno));
            close(fd_dev_mem);
            exit(EXIT_FAILURE);
        }

        hps_gpio = NULL;

    // munmap fpga peripherals
	alt_write_word(h2p_ctrl_out_addr, ctrl_out);	// write down the control

	if (munmap(h2f_lw_axi_master, h2f_lw_axi_master_span) != 0) {
		printf("Error: h2f_lw_axi_master munmap() failed\n");
		printf("    errno = %s\n", strerror(errno));
		close(fd_dev_mem);
		exit(EXIT_FAILURE);
	}

	h2f_lw_axi_master	= NULL;
	h2p_led_addr		= NULL;
	h2p_sw_addr			= NULL;

	printf("\nNMR SYSTEM STOPS!\n");
}

void setup_hps_gpio() {
    // Initialize the HPS PIO controller:
    //     Set the direction of the HPS_LED GPIO bit to "output"
    //     Set the direction of the HPS_KEY_N GPIO bit to "input"
    void *hps_gpio_direction = ALT_GPIO_SWPORTA_DDR_ADDR(hps_gpio);
    alt_setbits_word(hps_gpio_direction, ALT_GPIO_PIN_OUTPUT << HPS_LED_PORT_BIT);
    alt_setbits_word(hps_gpio_direction, ALT_GPIO_PIN_INPUT << HPS_KEY_N_PORT_BIT);
}

void setup_fpga_leds() {
    // Switch on first LED only
    alt_write_word(h2p_led_addr, 0xF0);
}

void handle_hps_led() {
    void *hps_gpio_data = ALT_GPIO_SWPORTA_DR_ADDR(hps_gpio);
    void *hps_gpio_port = ALT_GPIO_EXT_PORTA_ADDR(hps_gpio);

    uint32_t hps_gpio_input = alt_read_word(hps_gpio_port) & HPS_KEY_N_MASK;

    // HPS_KEY_N is active-low
    bool toggle_hps_led = (~hps_gpio_input & HPS_KEY_N_MASK);

    if (toggle_hps_led) {
        uint32_t hps_led_value = alt_read_word(hps_gpio_data);
        hps_led_value >>= HPS_LED_PORT_BIT;
        hps_led_value = !hps_led_value;
        hps_led_value <<= HPS_LED_PORT_BIT;
        alt_replbits_word(hps_gpio_data, HPS_LED_MASK, hps_led_value);
    }
}

void handle_fpga_leds() {
    uint32_t leds_mask = alt_read_word(h2p_led_addr);

    if (leds_mask != (0x01 << (LED_DATA_WIDTH - 1))) {
        // rotate leds
        leds_mask <<= 1;
    } else {
        // reset leds
        leds_mask = 0x1;
    }

    alt_write_word(h2p_led_addr, leds_mask);

}

void create_measurement_folder(char * foldertype) {
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	char command[60];
	sprintf(foldername,"%s_%04d_%02d_%02d_%02d_%02d_%02d",foldertype,tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	sprintf(command,"mkdir %s",foldername);
	system(command);

	// copy the executable file to the folder
	sprintf(command,"cp ./eecs397_nmr_c %s/eecs397_nmr_c",foldername);
	system(command);
}

void CPMG_Sequence (
	double cpmg_freq,
	double pulse1_us,
	double pulse2_us,
	double pulse1_dtcl,
	double pulse2_dtcl,
	double echo_spacing_us,
	long unsigned scan_spacing_us,
	unsigned int samples_per_echo,
	unsigned int echoes_per_scan,
	double init_adc_delay_compensation,
	uint32_t ph_cycl_en,
	char * filename,
	char * avgname,
	uint32_t enable_message
){

	unsigned int cpmg_param [5];
	double adc_freq = 4.5;
	double adc_pll_freq = adc_freq/4.5*175;

	// local variables
	uint32_t fifo_mem_level; // the fill level of fifo memory

	// print starting fill memory number (to check the integrity of fifo)
	// fifo_mem_level = alt_read_word(h2p_adc_fifo_status_addr+ALTERA_AVALON_FIFO_LEVEL_REG); // the fill level of FIFO memory
	// printf("fifomem_lvl start: %6d\n",fifo_mem_level);

	cpmg_param_calculator_ltc2314(
		cpmg_param,
		cpmg_freq,
		(double)adc_freq,
		(double)adc_pll_freq,
		(double)init_adc_delay_compensation,
		pulse1_us,
		pulse2_us,
		echo_spacing_us,
		samples_per_echo
	);

	alt_write_word( (h2p_pulse1_addr) , cpmg_param[PULSE1_OFFST] );
	alt_write_word( (h2p_delay1_addr) , cpmg_param[DELAY1_OFFST] );
	alt_write_word( (h2p_pulse2_addr) , cpmg_param[PULSE2_OFFST] );
	alt_write_word( (h2p_delay2_addr) , cpmg_param[DELAY2_OFFST] );
	alt_write_word( (h2p_init_adc_delay_addr) , cpmg_param[INIT_DELAY_ADC_OFFST] );
	alt_write_word( (h2p_echo_per_scan_addr) , echoes_per_scan );
	alt_write_word( (h2p_adc_samples_per_echo_addr) , samples_per_echo );

	if (enable_message) {
		printf("CPMG Sequence Run Parameters:\n");
		printf("\tPulse 1\t\t\t: %7.3f us\n", (double)cpmg_param[PULSE1_OFFST]/cpmg_freq);
		printf("\tDelay 1\t\t\t: %7.3f us\n", (double)cpmg_param[DELAY1_OFFST]/cpmg_freq);
		printf("\tPulse 2\t\t\t: %7.3f us\n", (double)cpmg_param[PULSE2_OFFST]/cpmg_freq);
		printf("\tDelay 2\t\t\t: %7.3f us\n", (double)cpmg_param[DELAY2_OFFST]/cpmg_freq);
		printf("\tAcqu. wait\t\t: %7.3f us\n", cpmg_param[INIT_DELAY_ADC_OFFST]/adc_pll_freq);
		printf("\tAcqu. window\t: %7.3f us\n", ((double)samples_per_echo)/adc_freq);
	}
	if (cpmg_param[INIT_DELAY_ADC_OFFST] == 0) {
		printf("\tWARNING: ADC init delay is 0!");
	}

	// set pll for CPMG
	Set_PLL (h2p_nmr_pll_addr, 0, cpmg_freq, pulse2_dtcl, enable_message);
	Set_PLL (h2p_nmr_pll_addr, 1, cpmg_freq, pulse1_dtcl, enable_message);
	Set_PLL (h2p_nmr_pll_addr, 2, cpmg_freq, pulse2_dtcl, DISABLE_MESSAGE);
	Set_PLL (h2p_nmr_pll_addr, 3, cpmg_freq, pulse1_dtcl, DISABLE_MESSAGE);
	Reset_PLL (h2p_ctrl_out_addr, pll_nmr_rst_ofst, ctrl_out);
	Set_DPS (h2p_nmr_pll_addr, 0, 0, DISABLE_MESSAGE);
	Set_DPS (h2p_nmr_pll_addr, 1, 90, DISABLE_MESSAGE);
	Set_DPS (h2p_nmr_pll_addr, 2, 180, DISABLE_MESSAGE);
	Set_DPS (h2p_nmr_pll_addr, 3, 270, DISABLE_MESSAGE);
	Wait_PLL_To_Lock (h2p_ctrl_in_addr, pll_nmr_lock_ofst);

	// set pll for ADC
	Set_PLL (h2p_adc_pll_addr, 0, adc_pll_freq, (double)0.50, DISABLE_MESSAGE);
	Reset_PLL (h2p_ctrl_out_addr, pll_adc_rst_ofst, ctrl_out);
	Wait_PLL_To_Lock (h2p_ctrl_in_addr, pll_adc_lock_ofst);

	// print the NMR acquired settings
	sprintf(pathname,"%s/matlab_settings.txt",foldername);	// put the data into the data folder
	fptr = fopen(pathname, "a");
	fprintf(fptr,"%f\n", (double)cpmg_param[PULSE1_OFFST]/cpmg_freq);
	fprintf(fptr,"%f\n", (double)cpmg_param[DELAY1_OFFST]/cpmg_freq);
	fprintf(fptr,"%f\n", (double)cpmg_param[PULSE2_OFFST]/cpmg_freq);
	fprintf(fptr,"%f\n", (double)cpmg_param[DELAY2_OFFST]/cpmg_freq);
	fprintf(fptr,"%f\n", cpmg_param[INIT_DELAY_ADC_OFFST]/adc_pll_freq);
	fprintf(fptr,"%f\n",cpmg_freq);
	fprintf(fptr,"%f\n",pulse1_dtcl);
	fprintf(fptr,"%f\n",pulse2_dtcl);
	fprintf(fptr,"%f\n",echo_spacing_us);
	fprintf(fptr,"%lu\n",scan_spacing_us);
	fprintf(fptr,"%d\n",samples_per_echo);
	fprintf(fptr,"%d\n",echoes_per_scan);
	fclose(fptr);

	sprintf(pathname,"%s/matlab_settings_hr.txt",foldername);	// put the data into the data folder
	fptr = fopen(pathname, "a");
	fprintf(fptr,"Actual Pulse 1 Length: %f us\n", (double)cpmg_param[PULSE1_OFFST]/cpmg_freq);
	fprintf(fptr,"Actual Delay 1 Length: %f us\n", (double)cpmg_param[DELAY1_OFFST]/cpmg_freq);
	fprintf(fptr,"Actual Pulse 2 Length: %f us\n", (double)cpmg_param[PULSE2_OFFST]/cpmg_freq);
	fprintf(fptr,"Actual Delay 2 Length: %f us\n", (double)cpmg_param[DELAY2_OFFST]/cpmg_freq);
	fprintf(fptr,"Actual Init Delay Length: %f us\n", cpmg_param[INIT_DELAY_ADC_OFFST]/adc_pll_freq);
	fprintf(fptr,"Intended frequency : %f\n",cpmg_freq);
	fprintf(fptr,"Intended pulse 1 duty cycle : %f\n",pulse1_dtcl);
	fprintf(fptr,"Intended pulse 2 duty cycle : %f\n",pulse2_dtcl);
	fprintf(fptr,"Intended echo spacing : %f us\n",echo_spacing_us);
	fprintf(fptr,"Intended scan spacing : %lu us\n",scan_spacing_us);
	fprintf(fptr,"samples_per_echo : %d\n",samples_per_echo);
	fprintf(fptr,"echoes_per_scan : %d\n",echoes_per_scan);
	fclose(fptr);

	// cycle phase for CPMG measurement
	if (ph_cycl_en == ENABLE) {
		if (ctrl_out & (0x01<<phase_cycle_ofst)) {
			ctrl_out &= ~(0x01<<phase_cycle_ofst);
		}
		else {
			ctrl_out |= (0x01<<phase_cycle_ofst);
		}
		alt_write_word( (h2p_ctrl_out_addr) , ctrl_out );
		usleep(10);
	}


	// reset buffer
	ctrl_out |= (0x01<<adc_fifo_rst_ofst);
	alt_write_word( (h2p_ctrl_out_addr) , ctrl_out );
	usleep(10);
	ctrl_out &= ~(0x01<<adc_fifo_rst_ofst);
	alt_write_word( (h2p_ctrl_out_addr) , ctrl_out );
	usleep(10);

	// start fsm
	alt_write_word( (h2p_ctrl_out_addr) , ctrl_out | (0x01<<nmr_controller_start_ofst) );
	alt_write_word( (h2p_ctrl_out_addr) , ctrl_out & ~(0x01<<nmr_controller_start_ofst) );
	usleep(scan_spacing_us);

	// wait until fsm stops
	while ( alt_read_word(h2p_ctrl_in_addr) & (0x01<<NMR_controller_systemstat_ofst) );
	usleep(300);

	fifo_mem_level = alt_read_word(h2p_adc_fifo_status_addr+ALTERA_AVALON_FIFO_LEVEL_REG); // the fill level of FIFO memory
	for (i=0; fifo_mem_level>0; i++) {			// FIFO is 32-bit, while 1-sample is only 16-bit. FIFO organize this automatically. So, fetch only amount_of_data shifted by 2 to get amount_of_data/2.
		rddata[i] = alt_read_word(h2p_adc_fifo_addr);

		fifo_mem_level--;
		if (fifo_mem_level == 0) {
			fifo_mem_level = alt_read_word(h2p_adc_fifo_status_addr+ALTERA_AVALON_FIFO_LEVEL_REG);
		}
		//usleep(1);
	}
	usleep(100);

	// read the fifo level at the end of acquisition (debugging purpose)
	//fifo_mem_level = alt_read_word(h2p_adc_fifo_status_addr+ALTERA_AVALON_FIFO_LEVEL_REG);
	//printf("fifomem_lvl stop: %6d  requested data: %6d  iteration: %ld(*2)\n",fifo_mem_level,samples_per_echo*echoes_per_scan,i);




	if (i*2 == samples_per_echo*echoes_per_scan) { // if the amount of data captured matched with the amount of data being ordered, then continue the process. if not, then don't process the datas (requesting empty data from the fifo will cause the FPGA to crash, so this one is to avoid that)
		// printf("number of captured data vs requested data : MATCHED\n");

		j=0;
		for(i=0; i < ( ((long)samples_per_echo*(long)echoes_per_scan)>>1 ); i++) {
			rddata_16[j++] = (rddata[i] & 0x3FFF);		// 14 significant bit
			rddata_16[j++] = ((rddata[i]>>16)&0x3FFF);	// 14 significant bit
		}

		// write the raw data from adc to a file
		sprintf(pathname,"%s/%s",foldername,filename);	// put the data into the data folder
		fptr = fopen(pathname, "w");
		if (fptr == NULL) {
			printf("File does not exists \n");
		}
		for(i=0; i < ( ((long)samples_per_echo*(long)echoes_per_scan) ); i++) {
			fprintf(fptr, "%d\n", rddata_16[i]);
		}
		fclose(fptr);


		// write the averaged data to a file
		unsigned int avr_data[samples_per_echo];
		// initialize array
		for (i=0; i<samples_per_echo; i++) {
			avr_data[i] = 0;
		};
		for (i=0; i<samples_per_echo; i++) {
			for (j=i; j<( ((long)samples_per_echo*(long)echoes_per_scan) ); j+=samples_per_echo) {
				avr_data[i] += rddata_16[j];
			}
		}
		sprintf(pathname,"%s/%s",foldername,avgname);	// put the data into the data folder
		fptr = fopen(pathname, "w");
		if (fptr == NULL) {
			printf("File does not exists \n");
		}
		for (i=0; i<samples_per_echo; i++) {
			fprintf(fptr, "%d\n", avr_data[i]);
		}
		fclose(fptr);



	// DON'T DELETE
	//	fprintf(fptr, "CPMG Frequency: %3.3f MHz\n", cpmg_freq);
	//	fprintf(fptr, "Pulse 1 Length: %2.2f us\n", pulse1_us);
	//	fprintf(fptr, "Pulse 2 Length: %2.2f us\n", pulse2_us);
	//	fprintf(fptr, "Pulse 1 Duty Cycle: %1.3f %%\n", pulse1_dtcl);
	//	fprintf(fptr, "Pulse 2 Duty Cycle: %1.3f %%\n", pulse2_dtcl);
	//	fprintf(fptr, "Echotime: %3.2f us\n", echo_spacing_us);
	//	fprintf(fptr, "Total Sample: %d\n", samples_per_echo);
	//	fprintf(fptr, "Capture Amount: %d\n\n", echoes_per_scan);

	}
	else { // if the amount of data captured didn't match the amount of data being ordered, then something's going on with the acquisition

		printf("number of data captured (%ld) and data ordered (%d): NOT MATCHED\nReconfigure the FPGA immediately\n", i*2, samples_per_echo*echoes_per_scan);
	}

}

void CPMG_iterate (
	double cpmg_freq,
	double pulse1_us,
	double pulse2_us,
	double pulse1_dtcl,
	double pulse2_dtcl,
	double echo_spacing_us,
	long unsigned scan_spacing_us,
	unsigned int samples_per_echo,
	unsigned int echoes_per_scan,
	double init_adc_delay_compensation,
	unsigned int number_of_iteration,
	uint32_t ph_cycl_en
){

	create_measurement_folder("nmr");

	// print general measurement settings
	sprintf(pathname,"%s/CPMG_iterate_settings.txt",foldername);
	fptr = fopen(pathname, "a");
	fprintf(fptr,"cpmg_freq:\t%f\n", cpmg_freq);
	fprintf(fptr,"pulse1_us:\t%f\n", pulse1_us);
	fprintf(fptr,"pulse2_us:\t%f\n", pulse2_us);
	fprintf(fptr,"pulse1_dtcl:\t%f\n", pulse1_dtcl);
	fprintf(fptr,"pulse2_dtcl:\t%f\n", pulse2_dtcl);
	fprintf(fptr,"echo_spacing_us:\t%f\n", echo_spacing_us);
	fprintf(fptr,"scan_spacing_us:\t%lu\n", scan_spacing_us);
	fprintf(fptr,"samples_per_echo:\t%d\n", samples_per_echo);
	fprintf(fptr,"echoes_per_scan:\t%d\n", echoes_per_scan);
	fprintf(fptr,"init_adc_delay_compensation:\t%f\n", init_adc_delay_compensation);
	fprintf(fptr,"number_of_iteration:\t%d\n", number_of_iteration);
	if (ph_cycl_en == ENABLE) {
		fprintf(fptr,"phase_cycling:\tyes\n");
	}
	else {
		fprintf(fptr,"phase_cycling:\tno\n");
	}
	fclose(fptr);

	// print matlab script to analyze datas
	sprintf(pathname,"measurement_history_matlab_script.txt");
	fptr = fopen(pathname, "a");
	fprintf(fptr,"compute_iterate([data_folder,'%s'],%d);\n",foldername,number_of_iteration);
	fclose(fptr);

	int iterate = 0;

	int FILENAME_LENGTH = 100;
	char *name;
	name = (char*) malloc (FILENAME_LENGTH*sizeof(char));
	char *nameavg;
	nameavg = (char*) malloc (FILENAME_LENGTH*sizeof(char));

	for (iterate=0; iterate<=number_of_iteration; iterate++) {
		printf("\n*** RUN %d ***\n",iterate);

		snprintf(name, FILENAME_LENGTH,"dat_%03d",iterate);
		snprintf(nameavg, FILENAME_LENGTH,"avg_%03d",iterate);

		CPMG_Sequence (
			cpmg_freq,						//cpmg_freq
			pulse1_us,						//pulse1_us
			pulse2_us,						//pulse2_us
			pulse1_dtcl,					//pulse1_dtcl
			pulse2_dtcl,					//pulse2_dtcl
			echo_spacing_us,				//echo_spacing_us
			scan_spacing_us,				//scan_spacing_us
			samples_per_echo,				//samples_per_echo
			echoes_per_scan,				//echoes_per_scan
			init_adc_delay_compensation,	//compensation delay number (counted by the adc base clock)
			ph_cycl_en,						//phase cycle enable/disable
			name,							//filename for data
			nameavg,						//filename for average data
			ENABLE_MESSAGE
		);

	}

	free(name);
	free(nameavg);

}

void CPMG_freq_sweep (
	double cpmg_freq_start,
	double cpmg_freq_stop,
	double cpmg_freq_spacing,
	double pulse1_us,
	double pulse2_us,
	double pulse1_dtcl,
	double pulse2_dtcl,
	double echo_spacing_us,
	long unsigned scan_spacing_us,
	unsigned int samples_per_echo,
	unsigned int echoes_per_scan,
	double init_adc_delay_compensation,
	unsigned int number_of_iteration,
	uint32_t ph_cycl_en
){

	create_measurement_folder("nmr_freqsweep");

	// print general measurement settings
	sprintf(pathname,"%s/CPMG_freq_sweep_settings.txt",foldername);
	fptr = fopen(pathname, "a");
	fprintf(fptr,"cpmg_freq_start:\t%f\n", cpmg_freq_start);
	fprintf(fptr,"cpmg_freq_stop:\t%f\n", cpmg_freq_stop);
	fprintf(fptr,"cpmg_freq_spacing:\t%f\n", cpmg_freq_spacing);
	fprintf(fptr,"pulse1_us:\t%f\n", pulse1_us);
	fprintf(fptr,"pulse2_us:\t%f\n", pulse2_us);
	fprintf(fptr,"pulse1_dtcl:\t%f\n", pulse1_dtcl);
	fprintf(fptr,"pulse2_dtcl:\t%f\n", pulse2_dtcl);
	fprintf(fptr,"echo_spacing_us:\t%f\n", echo_spacing_us);
	fprintf(fptr,"scan_spacing_us:\t%lu\n", scan_spacing_us);
	fprintf(fptr,"samples_per_echo:\t%d\n", samples_per_echo);
	fprintf(fptr,"echoes_per_scan:\t%d\n", echoes_per_scan);
	fprintf(fptr,"init_adc_delay_compensation:\t%f\n", init_adc_delay_compensation);
	if (ph_cycl_en == ENABLE) {
		fprintf(fptr,"phase_cycling:\tyes\n");
	}
	else {
		fprintf(fptr,"phase_cycling:\tno\n");
	}
	fclose(fptr);

	// print matlab script to analyze datas
	sprintf(pathname,"measurement_history_matlab_script.txt");
	fptr = fopen(pathname, "a");
	fprintf(fptr,"compute_freqsw([data_folder,'%s'],%d,0);\n",foldername,number_of_iteration);
	fclose(fptr);

	// print the NMR frequency sweep settings
	sprintf(pathname,"%s/freq_sweep.txt",foldername);	// put the data into the data folder
	fptr = fopen(pathname, "a");
	fprintf(fptr,"%f\n",cpmg_freq_start);
	fprintf(fptr,"%f\n",cpmg_freq_stop);
	fprintf(fptr,"%f\n",cpmg_freq_spacing);
	fclose(fptr);

	double cpmg_freq = 0;
	int FILENAME_LENGTH = 100;
	char *name;
	name = (char*) malloc (FILENAME_LENGTH*sizeof(char));
	char *nameavg;
	nameavg = (char*) malloc (FILENAME_LENGTH*sizeof(char));
	unsigned int iterate;
	for (cpmg_freq=cpmg_freq_start; cpmg_freq<=cpmg_freq_stop; cpmg_freq+=cpmg_freq_spacing) {
		printf("\n*** current cpmg_freq %f ***\n",cpmg_freq);

		for (iterate = 1; iterate <= number_of_iteration; iterate++) {
			snprintf(name, FILENAME_LENGTH,"dat_%06.3f_%03d",cpmg_freq,iterate);
			snprintf(nameavg, FILENAME_LENGTH,"avg_%06.3f_%03d",cpmg_freq,iterate);

			CPMG_Sequence (
				(double)cpmg_freq,				//cpmg_freq
				(double)pulse1_us,				//pulse1_us
				(double)pulse2_us,				//pulse2_us
				(double)pulse1_dtcl,			//pulse1_dtcl
				(double)pulse2_dtcl,			//pulse2_dtcl
				(double)echo_spacing_us,		//echo_spacing_us
				scan_spacing_us,				//scan_spacing_us
				samples_per_echo,				//samples_per_echo
				echoes_per_scan,				//echoes_per_scan
				init_adc_delay_compensation,	//compensation delay number (counted by the adc base clock)
				ph_cycl_en,						//phase cycle enable/disable
				name,							//filename for data
				nameavg,						//filename for average data
				DISABLE_MESSAGE
			);

		}

	}

	free(name);
	free(nameavg);

}

void CPMG_amp_dt_sweep (
	double pulse_dtcl_start,
	double pulse_dtcl_stop,
	double pulse_dtcl_spacing,
	double cpmg_freq,
	double pulse1_us,
	double pulse2_us,
	// double pulse2_dtcl,
	double echo_spacing_us,
	long unsigned scan_spacing_us,
	unsigned int samples_per_echo,
	unsigned int echoes_per_scan,
	double init_adc_delay_compensation,
	unsigned int number_of_iteration,
	uint32_t ph_cycl_en
){

	create_measurement_folder("nmr_amp_dt1_sweep");

	// print general measurement settings
	sprintf(pathname,"%s/CPMG_amp_dt1_sweep_settings.txt",foldername);
	fptr = fopen(pathname, "a");
	fprintf(fptr,"cpmg_freq:\t%f\n", cpmg_freq);
	fprintf(fptr,"pulse1_us:\t%f\n", pulse1_us);
	fprintf(fptr,"pulse2_us:\t%f\n", pulse2_us);
	fprintf(fptr,"pulse1_dtcl_start:\t%f\n", pulse_dtcl_start);
	fprintf(fptr,"pulse1_dtcl_stop:\t%f\n", pulse_dtcl_stop);
	fprintf(fptr,"pulse1_dtcl_spacing:\t%f\n", pulse_dtcl_spacing);
	// fprintf(fptr,"pulse2_dtcl:\t%f\n", pulse2_dtcl);
	fprintf(fptr,"echo_spacing_us:\t%f\n", echo_spacing_us);
	fprintf(fptr,"scan_spacing_us:\t%lu\n", scan_spacing_us);
	fprintf(fptr,"samples_per_echo:\t%d\n", samples_per_echo);
	fprintf(fptr,"echoes_per_scan:\t%d\n", echoes_per_scan);
	fprintf(fptr,"init_adc_delay_compensation:\t%f\n", init_adc_delay_compensation);
	if (ph_cycl_en == ENABLE) {
		fprintf(fptr,"phase_cycling:\tyes\n");
	}
	else {
		fprintf(fptr,"phase_cycling:\tno\n");
	}
	fclose(fptr);

	// print matlab script to analyze datas
	sprintf(pathname,"measurement_history_matlab_script.txt");
	fptr = fopen(pathname, "a");
	fprintf(fptr,"compute_dt1sw([data_folder,'%s'],%d,0);\n",foldername,number_of_iteration);
	fclose(fptr);

	// print the NMR frequency sweep settings
	sprintf(pathname,"%s/dt1_sweep.txt",foldername);	// put the data into the data folder
	fptr = fopen(pathname, "a");
	fprintf(fptr,"%f\n",pulse_dtcl_start);
	fprintf(fptr,"%f\n",pulse_dtcl_stop);
	fprintf(fptr,"%f\n",pulse_dtcl_spacing);
	fclose(fptr);

	double pulse_dtcl = 0;
	int FILENAME_LENGTH = 100;
	char *name;
	name = (char*) malloc (FILENAME_LENGTH*sizeof(char));
	char *nameavg;
	nameavg = (char*) malloc (FILENAME_LENGTH*sizeof(char));
	unsigned int iterate;

	for (pulse_dtcl=pulse_dtcl_start; pulse_dtcl<=pulse_dtcl_stop; pulse_dtcl+=pulse_dtcl_spacing) {
		printf("\n*** current pulse_dtcl %f ***\n",pulse_dtcl);

		for (iterate = 1; iterate <= number_of_iteration; iterate++) {

			snprintf(name, FILENAME_LENGTH,"dat_%03.3f_%03d",pulse_dtcl,iterate);
			snprintf(nameavg, FILENAME_LENGTH,"avg_%03.3f_%03d",pulse_dtcl,iterate);
			CPMG_Sequence (
				(double)cpmg_freq,				//cpmg_freq
				(double)pulse1_us,				//pulse1_us
				(double)pulse2_us,				//pulse2_us
				(double)pulse_dtcl,				//pulse1_dtcl
				(double)pulse_dtcl,				//pulse2_dtcl
				(double)echo_spacing_us,		//echo_spacing_us
				scan_spacing_us,				//scan_spacing_us
				samples_per_echo,				//samples_per_echo
				echoes_per_scan,				//echoes_per_scan
				init_adc_delay_compensation,	//compensation delay number (counted by the adc base clock)
				ph_cycl_en,						//phase cycle enable/disable
				name,							//filename for data
				nameavg,						//filename for average data
				DISABLE_MESSAGE
			);
		}

	}

	free(name);
	free(nameavg);
}

void CPMG_amp_pulse1_length_sweep (
	double pulse1_us_start,
	double pulse1_us_stop,
	double pulse1_us_spacing,
	double cpmg_freq,
	double pulse1_dtcl,
	double pulse2_us,
	double pulse2_dtcl,
	double echo_spacing_us,
	long unsigned scan_spacing_us,
	unsigned int samples_per_echo,
	unsigned int echoes_per_scan,
	double init_adc_delay_compensation,
	unsigned int number_of_iteration,
	uint32_t ph_cycl_en
){

	create_measurement_folder("nmr_amp_pulse1_length_sweep");

	// print general measurement settings
	sprintf(pathname,"%s/CPMG_amp_pulse1_length_sweep_settings.txt",foldername);
	fptr = fopen(pathname, "a");
	fprintf(fptr,"cpmg_freq:\t%f\n", cpmg_freq);
	fprintf(fptr,"pulse1_us_start:\t%f\n", pulse1_us_start);
	fprintf(fptr,"pulse1_us_stop:\t%f\n", pulse1_us_stop);
	fprintf(fptr,"pulse1_us_spacing:\t%f\n", pulse1_us_spacing);
	fprintf(fptr,"pulse2_us:\t%f\n", pulse2_us);
	fprintf(fptr,"pulse1_dtcl:\t%f\n", pulse1_dtcl);
	fprintf(fptr,"pulse2_dtcl:\t%f\n", pulse2_dtcl);
	fprintf(fptr,"echo_spacing_us:\t%f\n", echo_spacing_us);
	fprintf(fptr,"scan_spacing_us:\t%lu\n", scan_spacing_us);
	fprintf(fptr,"samples_per_echo:\t%d\n", samples_per_echo);
	fprintf(fptr,"echoes_per_scan:\t%d\n", echoes_per_scan);
	fprintf(fptr,"init_adc_delay_compensation:\t%f\n", init_adc_delay_compensation);
	if (ph_cycl_en == ENABLE) {
		fprintf(fptr,"phase_cycling:\tyes\n");
	}
	else {
		fprintf(fptr,"phase_cycling:\tno\n");
	}
	fclose(fptr);

	// print matlab script to analyze datas
	sprintf(pathname,"measurement_history_matlab_script.txt");
	fptr = fopen(pathname, "a");
	fprintf(fptr,"compute_p1lsw([data_folder,'%s'],%d,0);\n",foldername,number_of_iteration);
	fclose(fptr);

	// print the NMR frequency sweep settings
	sprintf(pathname,"%s/p1_length_sweep.txt",foldername);	// put the data into the data folder
	fptr = fopen(pathname, "a");
	fprintf(fptr,"%f\n",pulse1_us_start);
	fprintf(fptr,"%f\n",pulse1_us_stop);
	fprintf(fptr,"%f\n",pulse1_us_spacing);
	fclose(fptr);

	double pulse1_us = 0;
	int FILENAME_LENGTH = 100;
	char *name;
	name = (char*) malloc (FILENAME_LENGTH*sizeof(char));
	char *nameavg;
	nameavg = (char*) malloc (FILENAME_LENGTH*sizeof(char));
	unsigned int iterate;

	for (pulse1_us=pulse1_us_start; pulse1_us<=pulse1_us_stop; pulse1_us+=pulse1_us_spacing) {
		printf("\n*** current pulse1_us %f ***\n",pulse1_us);

		for (iterate = 1; iterate <= number_of_iteration; iterate++) {

			snprintf(name, FILENAME_LENGTH,"dat_%05.3f_%03d",pulse1_us,iterate);
			snprintf(nameavg, FILENAME_LENGTH,"avg_%05.3f_%03d",pulse1_us,iterate);

			CPMG_Sequence (
				(double)cpmg_freq,				//cpmg_freq
				(double)pulse1_us,				//pulse1_us
				(double)pulse2_us,				//pulse2_us
				(double)pulse1_dtcl,			//pulse1_dtcl
				(double)pulse2_dtcl,			//pulse2_dtcl
				(double)echo_spacing_us,		//echo_spacing_us
				scan_spacing_us,				//scan_spacing_us
				samples_per_echo,				//samples_per_echo
				echoes_per_scan,				//echoes_per_scan
				init_adc_delay_compensation,	//compensation delay number (counted by the adc base clock)
				ph_cycl_en,						//phase cycle enable/disable
				name,							//filename for data
				nameavg,						//filename for average data
				DISABLE_MESSAGE
			);

		}

	}

	free(name);
	free(nameavg);

}

void CPMG_amp_pulse2_length_sweep (
	double pulse2_us_start,
	double pulse2_us_stop,
	double pulse2_us_spacing,
	double cpmg_freq,
	double pulse1_us,
	double pulse1_dtcl,
	double pulse2_dtcl,
	double echo_spacing_us,
	long unsigned scan_spacing_us,
	unsigned int samples_per_echo,
	unsigned int echoes_per_scan,
	double init_adc_delay_compensation,
	unsigned int number_of_iteration,
	uint32_t ph_cycl_en
){

	create_measurement_folder("nmr_amp_pulse2_length_sweep");

	// print general measurement settings
	sprintf(pathname,"%s/CPMG_amp_pulse2_length_sweep_settings.txt",foldername);
	fptr = fopen(pathname, "a");
	fprintf(fptr,"cpmg_freq:\t%f\n", cpmg_freq);
	fprintf(fptr,"pulse1_us:\t%f\n", pulse1_us);
	fprintf(fptr,"pulse2_us_start:\t%f\n", pulse2_us_start);
	fprintf(fptr,"pulse2_us_stop:\t%f\n", pulse2_us_stop);
	fprintf(fptr,"pulse2_us_spacing:\t%f\n", pulse2_us_spacing);
	fprintf(fptr,"pulse1_dtcl:\t%f\n", pulse1_dtcl);
	fprintf(fptr,"pulse2_dtcl:\t%f\n", pulse2_dtcl);
	fprintf(fptr,"echo_spacing_us:\t%f\n", echo_spacing_us);
	fprintf(fptr,"scan_spacing_us:\t%lu\n", scan_spacing_us);
	fprintf(fptr,"samples_per_echo:\t%d\n", samples_per_echo);
	fprintf(fptr,"echoes_per_scan:\t%d\n", echoes_per_scan);
	fprintf(fptr,"init_adc_delay_compensation:\t%f\n", init_adc_delay_compensation);
	if (ph_cycl_en == ENABLE) {
		fprintf(fptr,"phase_cycling:\tyes\n");
	}
	else {
		fprintf(fptr,"phase_cycling:\tno\n");
	}
	fclose(fptr);

	// print matlab script to analyze datas
	sprintf(pathname,"measurement_history_matlab_script.txt");
	fptr = fopen(pathname, "a");
	fprintf(fptr,"compute_p2lsw([data_folder,'%s'],%d,0);\n",foldername,number_of_iteration);
	fclose(fptr);

	// print the NMR frequency sweep settings
	sprintf(pathname,"%s/p2_length_sweep.txt",foldername);	// put the data into the data folder
	fptr = fopen(pathname, "a");
	fprintf(fptr,"%f\n",pulse2_us_start);
	fprintf(fptr,"%f\n",pulse2_us_stop);
	fprintf(fptr,"%f\n",pulse2_us_spacing);
	fclose(fptr);

	double pulse2_us = 0;
	int FILENAME_LENGTH = 100;
	char *name;
	name = (char*) malloc (FILENAME_LENGTH*sizeof(char));
	char *nameavg;
	nameavg = (char*) malloc (FILENAME_LENGTH*sizeof(char));
	unsigned int iterate;

	for (pulse2_us=pulse2_us_start; pulse2_us<=pulse2_us_stop; pulse2_us+=pulse2_us_spacing) {
		printf("\n*** current pulse2_us %f ***\n",pulse2_us);

		for (iterate = 1; iterate <= number_of_iteration; iterate++) {

			snprintf(name, FILENAME_LENGTH,"dat_%05.3f_%03d",pulse2_us,iterate);
			snprintf(nameavg, FILENAME_LENGTH,"avg_%05.3f_%03d",pulse2_us,iterate);

			CPMG_Sequence (
				(double)cpmg_freq,				//cpmg_freq
				(double)pulse1_us,				//pulse1_us
				(double)pulse2_us,				//pulse2_us
				(double)pulse1_dtcl,			//pulse1_dtcl
				(double)pulse2_dtcl,			//pulse2_dtcl
				(double)echo_spacing_us,		//echo_spacing_us
				scan_spacing_us,				//scan_spacing_us
				samples_per_echo,				//samples_per_echo
				echoes_per_scan,				//echoes_per_scan
				init_adc_delay_compensation,	//compensation delay number (counted by the adc base clock)
				ph_cycl_en,						//phase cycle enable/disable
				name,							//filename for data
				nameavg,						//filename for average data
				DISABLE_MESSAGE
			);

		}

	}

	free(name);
	free(nameavg);

}

void CPMG_amp_length_sweep (
	double pulse_us_start,
	double pulse_us_stop,
	double pulse_us_spacing,
	double cpmg_freq,
	double pulse1_dtcl,
	double pulse2_dtcl,
	double echo_spacing_us,
	long unsigned scan_spacing_us,
	unsigned int samples_per_echo,
	unsigned int echoes_per_scan,
	double init_adc_delay_compensation,
	unsigned int number_of_iteration,
	uint32_t ph_cycl_en
){

	create_measurement_folder("nmr_amp_length_sweep");

	// print general measurement settings
	sprintf(pathname,"%s/CPMG_amp_length_sweep_settings.txt",foldername);
	fptr = fopen(pathname, "a");
	fprintf(fptr,"cpmg_freq:\t%f\n", cpmg_freq);
	fprintf(fptr,"pulse_us_start:\t%f\n", pulse_us_start);
	fprintf(fptr,"pulse_us_stop:\t%f\n", pulse_us_stop);
	fprintf(fptr,"pulse_us_spacing:\t%f\n", pulse_us_spacing);
	fprintf(fptr,"pulse1_dtcl:\t%f\n", pulse1_dtcl);
	fprintf(fptr,"pulse2_dtcl:\t%f\n", pulse2_dtcl);
	fprintf(fptr,"echo_spacing_us:\t%f\n", echo_spacing_us);
	fprintf(fptr,"scan_spacing_us:\t%lu\n", scan_spacing_us);
	fprintf(fptr,"samples_per_echo:\t%d\n", samples_per_echo);
	fprintf(fptr,"echoes_per_scan:\t%d\n", echoes_per_scan);
	fprintf(fptr,"init_adc_delay_compensation:\t%f\n", init_adc_delay_compensation);
	if (ph_cycl_en == ENABLE) {
		fprintf(fptr,"phase_cycling:\tyes\n");
	}
	else {
		fprintf(fptr,"phase_cycling:\tno\n");
	}
	fclose(fptr);

	// print matlab script to analyze datas
	sprintf(pathname,"measurement_history_matlab_script.txt");
	fptr = fopen(pathname, "a");
	fprintf(fptr,"compute_plsw([data_folder,'%s'],%d,0);\n",foldername,number_of_iteration);
	fclose(fptr);

	// print the NMR frequency sweep settings
	sprintf(pathname,"%s/p_length_sweep.txt",foldername);	// put the data into the data folder
	fptr = fopen(pathname, "a");
	fprintf(fptr,"%f\n",pulse_us_start);
	fprintf(fptr,"%f\n",pulse_us_stop);
	fprintf(fptr,"%f\n",pulse_us_spacing);
	fclose(fptr);

	double pulse_us = 0;
	int FILENAME_LENGTH = 100;
	char *name;
	name = (char*) malloc (FILENAME_LENGTH*sizeof(char));
	char *nameavg;
	nameavg = (char*) malloc (FILENAME_LENGTH*sizeof(char));
	unsigned int iterate;

	for (pulse_us=pulse_us_start; pulse_us<=pulse_us_stop; pulse_us+=pulse_us_spacing) {
		printf("\n*** current pulse2_us %f ***\n",pulse_us);

		for (iterate = 1; iterate <= number_of_iteration; iterate++) {

			snprintf(name, FILENAME_LENGTH,"dat_%05.3f_%03d",pulse_us,iterate);
			snprintf(nameavg, FILENAME_LENGTH,"avg_%05.3f_%03d",pulse_us,iterate);

			CPMG_Sequence (
				(double)cpmg_freq,				//cpmg_freq
				(double)pulse_us,				//pulse1_us
				(double)pulse_us*1.6,			//pulse2_us
				(double)pulse1_dtcl,			//pulse1_dtcl
				(double)pulse2_dtcl,			//pulse2_dtcl
				(double)echo_spacing_us,		//echo_spacing_us
				scan_spacing_us,				//scan_spacing_us
				samples_per_echo,				//samples_per_echo
				echoes_per_scan,				//echoes_per_scan
				init_adc_delay_compensation,	//compensation delay number (counted by the adc base clock)
				ph_cycl_en,						//phase cycle enable/disable
				name,							//filename for data
				nameavg,						//filename for average data
				DISABLE_MESSAGE
			);

		}

	}

	free(name);
	free(nameavg);

}

void test_leds_and_switches () {
    // initialize gpio for the hps
    setup_hps_gpio();

    // switch on first led
    setup_fpga_leds();


    while (true) {
        handle_hps_led();
        handle_fpga_leds();
        printf("%d\n",alt_read_word(h2p_sw_addr));
        usleep(ALT_MICROSECS_IN_A_SEC / 10);
    }
}

void CPMG_T1_meas (
	double cpmg_freq,
	double pulse180_t1_us,
	double delay180_t1_us_start,
	double delay180_t1_us_stop,
	double delay180_t1_us_spacing,
	double pulse1_us,
	double pulse2_us,
	double pulse1_dtcl,
	double pulse2_dtcl,
	double echo_spacing_us,
	long unsigned scan_spacing_us,
	unsigned int samples_per_echo,
	unsigned int echoes_per_scan,
	double init_adc_delay_compensation,
	unsigned int number_of_iteration,
	uint32_t ph_cycl_en
){

	double pulse180_t1_int;
	double delay180_t1_int;

	create_measurement_folder("nmr_t1_meas");

	// print general measurement settings
	sprintf(pathname,"%s/CPMG_t1_meas_settings.txt",foldername);
	fptr = fopen(pathname, "a");
	fprintf(fptr,"cpmg_freq:\t%f\n", cpmg_freq);
	fprintf(fptr,"pulse180_t1_us:\t%f\n", pulse180_t1_us);
	fprintf(fptr,"delay180_t1_us_start:\t%f\n", delay180_t1_us_start);
	fprintf(fptr,"delay180_t1_us_stop:\t%f\n", delay180_t1_us_stop);
	fprintf(fptr,"delay180_t1_us_spacing:\t%f\n", delay180_t1_us_spacing);
	fprintf(fptr,"pulse1_us:\t%f\n", pulse1_us);
	fprintf(fptr,"pulse2_us:\t%f\n", pulse2_us);
	fprintf(fptr,"pulse1_dtcl:\t%f\n", pulse1_dtcl);
	fprintf(fptr,"pulse2_dtcl:\t%f\n", pulse2_dtcl);
	fprintf(fptr,"echo_spacing_us:\t%f\n", echo_spacing_us);
	fprintf(fptr,"scan_spacing_us:\t%lu\n", scan_spacing_us);
	fprintf(fptr,"samples_per_echo:\t%d\n", samples_per_echo);
	fprintf(fptr,"echoes_per_scan:\t%d\n", echoes_per_scan);
	fprintf(fptr,"init_adc_delay_compensation:\t%f\n", init_adc_delay_compensation);
	if (ph_cycl_en == ENABLE) {
		fprintf(fptr,"phase_cycling:\tyes\n");
	}
	else {
		fprintf(fptr,"phase_cycling:\tno\n");
	}
	fclose(fptr);

	// print matlab script to analyze datas
	sprintf(pathname,"measurement_history_matlab_script.txt");
	fptr = fopen(pathname, "a");
	fprintf(fptr,"compute_t1_meas([data_folder,'%s'],%d,0);\n",foldername,number_of_iteration);
	fclose(fptr);

	// print the NMR frequency sweep settings
	sprintf(pathname,"%s/t1_meas.txt",foldername);	// put the data into the data folder
	fptr = fopen(pathname, "a");
	fprintf(fptr,"%f\n",delay180_t1_us_start);
	fprintf(fptr,"%f\n",delay180_t1_us_stop);
	fprintf(fptr,"%f\n",delay180_t1_us_spacing);
	fclose(fptr);

	double delay180_t1_us = 0;
	int FILENAME_LENGTH = 100;
	char *name;
	name = (char*) malloc (FILENAME_LENGTH*sizeof(char));
	char *nameavg;
	nameavg = (char*) malloc (FILENAME_LENGTH*sizeof(char));
	unsigned int iterate;

	for (delay180_t1_us=delay180_t1_us_start; delay180_t1_us<=delay180_t1_us_stop; delay180_t1_us+=delay180_t1_us_spacing) {
		printf("\n*** current delay180_us : %f us ***\n",delay180_t1_us);

		pulse180_t1_int = pulse180_t1_us * cpmg_freq;
		delay180_t1_int = delay180_t1_us * cpmg_freq - pulse180_t1_int;

		alt_write_word( h2p_pulse180_t1_addr , (unsigned int) pulse180_t1_int );
		alt_write_word( h2p_delay180_t1_addr , (unsigned int) delay180_t1_int );

		for (iterate = 1; iterate <= number_of_iteration; iterate++) {

			snprintf(name, FILENAME_LENGTH,"dat_%03.3f_%03d",delay180_t1_us,iterate);
			snprintf(nameavg, FILENAME_LENGTH,"avg_%03.3f_%03d",delay180_t1_us,iterate);
			CPMG_Sequence (
				(double)cpmg_freq,				//cpmg_freq
				(double)pulse1_us,				//pulse1_us
				(double)pulse2_us,				//pulse2_us
				(double)pulse1_dtcl,			//pulse1_dtcl
				(double)pulse2_dtcl,			//pulse2_dtcl
				(double)echo_spacing_us,		//echo_spacing_us
				scan_spacing_us,				//scan_spacing_us
				samples_per_echo,				//samples_per_echo
				echoes_per_scan,				//echoes_per_scan
				init_adc_delay_compensation,	//compensation delay number (counted by the adc base clock)
				ph_cycl_en,						//phase cycle enable/disable
				name,							//filename for data
				nameavg,						//filename for average data
				DISABLE_MESSAGE
			);
		}

		// RESET THE VALUE TO 0. WHICH BY DEFAULT SKIPPING THE T1 MEASUREMENT SEQUENCE
		alt_write_word( h2p_pulse180_t1_addr , 0 );
		alt_write_word( h2p_delay180_t1_addr , 0 );

	}

	free(name);
	free(nameavg);
}

int main() {
    // Initialize system
    init();

    // TEST COMMAND
    // test_leds_and_switches();

    // AgNext
    double cpmg_freq = 4.3;		// 4.297 by default
	double pulse1_us = 10;		// 7 by default
	double pulse2_us = 16;		// 7 by default
	double pulse1_dtcl = 0.1; 	// 0.065 by default
	double pulse2_dtcl = 0.1;	// 0.2 by default
	double echo_spacing_us = 300;
	unsigned int samples_per_echo = 256; //2048
	unsigned int echoes_per_scan = 512; //1024
	double init_adc_delay_compensation = 16;	// the delay to compensate signal shifting due to path delay in the AFE, in microseconds
	unsigned int number_of_iteration = 10;
	uint32_t ph_cycl_en = ENABLE;
	long unsigned scan_spacing_us_ref = 4000000;
	long unsigned scan_spacing_us_sample = 1000000;
	long unsigned scan_spacing_us;

    puts("Recallibrate with reference? Answer with 'y' or 'n'.\n");
    char comm;
    while (1) {
    	comm = getchar();
    	if (comm == 'n' || comm == 'y') {
    		break;
    	}
    	else {
    		while ((comm = getchar()) != '\n' && comm != EOF);
    		puts("Invalid answer. Please answer with 'y' or 'n'\n");
    	}
    }

    if (comm == 'y') { // put reference data
		while ((comm = getchar()) != '\n' && comm != EOF); // delete char buffer (to remove \n character)
    	puts("\n *** Place the reference. Press [ENTER] when done.\n");
    	getchar();
		scan_spacing_us = scan_spacing_us_ref; // 4000000
		CPMG_iterate (
			cpmg_freq,
			pulse1_us,
			pulse2_us,
			pulse1_dtcl,
			pulse2_dtcl,
			echo_spacing_us,
			scan_spacing_us,
			samples_per_echo,
			echoes_per_scan,
			init_adc_delay_compensation,
			number_of_iteration,
			ph_cycl_en
		);
		// print matlab script to analyze datas
		sprintf(pathname,"moisturetest_ref.txt");
		fptr = fopen(pathname, "w");
		fprintf(fptr,"%s,%d\n",foldername,number_of_iteration);
		fclose(fptr);
		sprintf(pathname,"moisturetest_ref_history.txt");
		fptr = fopen(pathname, "a");
		fprintf(fptr,"%s,%d\n",foldername,number_of_iteration);
		fclose(fptr);
    }

    if (comm == 'n'){ // delete char buffer: to fix glitch with the if-else above
    	while ((comm = getchar()) != '\n' && comm != EOF); // delete char buffer (to remove \n character)
    }
	puts("\n *** Place the sample. Press [ENTER] when done.\n");
	getchar();
	scan_spacing_us = scan_spacing_us_sample;
	CPMG_iterate (
		cpmg_freq,
		pulse1_us,
		pulse2_us,
		pulse1_dtcl,
		pulse2_dtcl,
		echo_spacing_us,
		scan_spacing_us,
		samples_per_echo,
		echoes_per_scan,
		init_adc_delay_compensation,
		number_of_iteration,
		ph_cycl_en
	);
	// print matlab script to analyze datas
	sprintf(pathname,"moisturetest_sample.txt");
	fptr = fopen(pathname, "w");
	fprintf(fptr,"%s,%d\n",foldername,number_of_iteration);
	fclose(fptr);
	sprintf(pathname,"moisturetest_sample_history.txt");
	fptr = fopen(pathname, "a");
	fprintf(fptr,"%s,%d\n",foldername,number_of_iteration);
	fclose(fptr);
    //


    /* CPMG ITERATE
	double cpmg_freq = 4.3;		// 4.297 by default
	double pulse1_us = 10;		// 7 by default
	double pulse2_us = 16;		// 7 by default
	double pulse1_dtcl = 0.1; 	// 0.065 by default
	double pulse2_dtcl = 0.1;	// 0.2 by default
	double echo_spacing_us = 300;
	long unsigned scan_spacing_us = 1000000; // 4000000
	unsigned int samples_per_echo = 256; //2048
	unsigned int echoes_per_scan = 512; //1024
	double init_adc_delay_compensation = 16;	// the delay to compensate signal shifting due to path delay in the AFE, in microseconds
	unsigned int number_of_iteration = 50;
	uint32_t ph_cycl_en = ENABLE;
	CPMG_iterate (
		cpmg_freq,
		pulse1_us,
		pulse2_us,
		pulse1_dtcl,
		pulse2_dtcl,
		echo_spacing_us,
		scan_spacing_us,
		samples_per_echo,
		echoes_per_scan,
		init_adc_delay_compensation,
		number_of_iteration,
		ph_cycl_en
	);
	*/

	/* T1 MEASUREMENT
	double cpmg_freq = 4.3;					// 4.297 by default
	double pulse180_t1_us = 16;
	double delay180_t1_us_start		= 100000;
	double delay180_t1_us_stop		= 2000000;
	double delay180_t1_us_spacing	= 100000;
	double pulse1_us = 10;					// 7 by default
	double pulse2_us = 16;					// 7 by default
	double pulse1_dtcl = 0.1; 				// 0.065 by default
	double pulse2_dtcl = 0.1;				// 0.2 by default
	double echo_spacing_us = 200;
	long unsigned scan_spacing_us = 1000000; // 4000000
	unsigned int samples_per_echo = 256; //2048
	unsigned int echoes_per_scan = 512; //1024
	double init_adc_delay_compensation = 6;	// the delay to compensate signal shifting due to path delay in the AFE, in microseconds
	unsigned int number_of_iteration = 2;
	uint32_t ph_cycl_en = ENABLE;
	CPMG_T1_meas (
		cpmg_freq,
		pulse180_t1_us,
		delay180_t1_us_start,
		delay180_t1_us_stop,
		delay180_t1_us_spacing,
		pulse1_us,
		pulse2_us,
		pulse1_dtcl,
		pulse2_dtcl,
		echo_spacing_us,
		scan_spacing_us,
		samples_per_echo,
		echoes_per_scan,
		init_adc_delay_compensation,
		number_of_iteration,
		ph_cycl_en
	);
	*/

    /* FREQUENCY SWEEP (HARDWARE TUNING IS LIMITED BY THE NMR_TABLE FREQ SPACING)
	double cpmg_freq_start = 4.2;
	double cpmg_freq_stop = 4.4;
	double cpmg_freq_spacing = 0.01; // the minimum is 20kHz spacing or 0.02. Don't know why it can't go less than that. The signal seems to be gone if it's less than that. The transmitter signal looks fine, but the signal received looks nothing.
	double pulse1_us = 15; // 5.8
	double pulse2_us = pulse1_us; // 9.3
	double pulse1_dtcl = 0.065;
	double pulse2_dtcl = 0.2;
	double echo_spacing_us = 500;
	long unsigned scan_spacing_us = 2000000;
	unsigned int samples_per_echo = 256;
	unsigned int echoes_per_scan = 512;
	double init_adc_delay_compensation = 6;	// the delay to compensate signal shifting due to path delay in the AFE, in microseconds
	unsigned int number_of_iteration = 4;
	uint32_t ph_cycl_en = ENABLE;
	CPMG_freq_sweep (
		cpmg_freq_start,
		cpmg_freq_stop,
		cpmg_freq_spacing,
		pulse1_us,
		pulse2_us,
		pulse1_dtcl,
		pulse2_dtcl,
		echo_spacing_us,
		scan_spacing_us,
		samples_per_echo,
		echoes_per_scan,
		init_adc_delay_compensation,
		number_of_iteration,
		ph_cycl_en
	);
	*/

    /* PULSE DTCL SWEEP
	double pulse_dtcl_start = 0.02;
	double pulse_dtcl_stop = 0.50;
	double pulse_dtcl_spacing = 0.02;
	double cpmg_freq = 4.3; tune_board(cpmg_freq);
	double pulse1_us = 20;
	double pulse2_us = 32;
	// double pulse2_dtcl = 0.35;
	double echo_spacing_us = 180;
	long unsigned scan_spacing_us = 400000;
	unsigned int samples_per_echo = 1024;
	unsigned int echoes_per_scan = 128;
	double init_adc_delay_compensation = 6;	// the delay to compensate signal shifting due to path delay in the AFE, in microseconds
	unsigned int number_of_iteration = 1;
	uint32_t ph_cycl_en = ENABLE;
	CPMG_amp_dt_sweep (
		pulse_dtcl_start,
		pulse_dtcl_stop,
		pulse_dtcl_spacing,
		cpmg_freq,
		pulse1_us,
		pulse2_us,
		echo_spacing_us,
		scan_spacing_us,
		samples_per_echo,
		echoes_per_scan,
		init_adc_delay_compensation,
		number_of_iteration,
		ph_cycl_en
	);
	*/

    /* PULSE1 LENGTH SWEEP
	double pulse1_us_start = 2;
	double pulse1_us_stop = 20;
	double pulse1_us_spacing = 1;
	double cpmg_freq = 4.3;
	double pulse1_dtcl = 0.1;
	double pulse2_us = 15;
	double pulse2_dtcl = 0.2;
	double echo_spacing_us = 500;
	long unsigned scan_spacing_us = 2000000;
	unsigned int samples_per_echo = 256;
	unsigned int echoes_per_scan = 512;
	double init_adc_delay_compensation = 6;	// the delay to compensate signal shifting due to path delay in the AFE, in microseconds
	unsigned int number_of_iteration = 4;
	uint32_t ph_cycl_en = ENABLE;
	CPMG_amp_pulse1_length_sweep (
		pulse1_us_start,
		pulse1_us_stop,
		pulse1_us_spacing,
		cpmg_freq,
		pulse1_dtcl,
		pulse2_us,
		pulse2_dtcl,
		echo_spacing_us,
		scan_spacing_us,
		samples_per_echo,
		echoes_per_scan,
		init_adc_delay_compensation,
		number_of_iteration,
		ph_cycl_en
	);
	*/

    /* PULSE2 LENGTH SWEEP
	double pulse2_us_start = 10;
	double pulse2_us_stop = 30;
	double pulse2_us_spacing = 1;
	double cpmg_freq = 4.3;
	double pulse1_dtcl = 0.1;
	double pulse1_us = 10;
	double pulse2_dtcl = 0.1;
	double echo_spacing_us = 500;
	long unsigned scan_spacing_us = 2000000;
	unsigned int samples_per_echo = 256;
	unsigned int echoes_per_scan = 512;
	double init_adc_delay_compensation = 6;	// the delay to compensate signal shifting due to path delay in the AFE, in microseconds
	unsigned int number_of_iteration = 4;
	uint32_t ph_cycl_en = ENABLE;
	CPMG_amp_pulse2_length_sweep (
		pulse2_us_start,
		pulse2_us_stop,
		pulse2_us_spacing,
		cpmg_freq,
		pulse1_us,
		pulse1_dtcl,
		pulse2_dtcl,
		echo_spacing_us,
		scan_spacing_us,
		samples_per_echo,
		echoes_per_scan,
		init_adc_delay_compensation,
		number_of_iteration,
		ph_cycl_en
	);
	*/

    /* CPMG PULSE LENGTH SWEEP (1.6 fixed ratio)
   	double pulse_us_start = 0.2;
   	double pulse_us_stop = 30;
   	double pulse_us_spacing = 0.2;
   	double cpmg_freq = 4.3; tune_board(cpmg_freq);
   	double pulse1_dtcl = 0.35;
   	double pulse2_dtcl = 0.35;
   	double echo_spacing_us = 200;
   	long unsigned scan_spacing_us = 400000;
   	unsigned int samples_per_echo = 2048;
   	unsigned int echoes_per_scan = 64;
   	double init_adc_delay_compensation = 6;	// the delay to compensate signal shifting due to path delay in the AFE, in microseconds
	unsigned int number_of_iteration = 4;
   	uint32_t ph_cycl_en = ENABLE;
   	CPMG_amp_length_sweep (
   		pulse_us_start,
   		pulse_us_stop,
   		pulse_us_spacing,
   		cpmg_freq,
   		pulse1_dtcl,
   		pulse2_dtcl,
   		echo_spacing_us,
   		scan_spacing_us,
   		samples_per_echo,
   		echoes_per_scan,
   		init_adc_delay_compensation,
   		number_of_iteration,
   		ph_cycl_en
   	);
	*/

    // exit program
    leave();

    return 0;
}
