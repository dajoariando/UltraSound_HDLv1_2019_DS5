// PUT ATTENTION to adc_data in read_adc_val function. It really depends on implementation of verilog
// It was found that the data captured by signal tap logic analyzer is delayed by 2 DCO clock cycles, therefore data needs to be shifted by two
// This might not be the case with different FPGA implementation!

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

#include <alt_generalpurpose_io.h>
#include <hwlib.h>
#include <socal/alt_gpio.h>
#include <socal/hps.h>
#include <socal/socal.h>
#include "functions/avalon_spi.h"

#include "hps_linux.h"
#include "./soc_variables/soc_system.h"
#include "./soc_variables/general.h"
#include "./soc_variables/lm96570_vars.h"
#include "./soc_variables/ad9276_vars.h"
#include "functions/AlteraIP/altera_avalon_fifo_regs.h"

// parameters
unsigned int num_of_samples = 100;
const unsigned int num_of_switches = 11;
const unsigned int num_of_channels = 8;

extern int fd_dev_mem;

unsigned int cnt_out_val;

void create_measurement_folder(char * foldertype) {
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	char command[60];
	sprintf(foldername,"%s_%04d_%02d_%02d_%02d_%02d_%02d",foldertype,tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	sprintf(command,"mkdir %s",foldername);
	system(command);

	// copy the executable file to the folder
	sprintf(command,"cp ./thesis_nmr_de1soc_hdl2.0 %s/execfile",foldername);
	system(command);
}

void init() {
	printf("ULTRASOUND SYSTEM STARTS!\n");

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

	//h2p_fifo_sink_csr_addr		= h2f_lw_axi_master + FIFO_SINK_OUT_CSR_BASE; // SINK_IN or SINK_OUT??????
	//h2p_fifo_sink_data_addr		= h2f_lw_axi_master + FIFO_SINK_OUT_BASE;
	h2p_fifo_sink_ch_a_csr_addr		= h2f_lw_axi_master + FIFO_SINK_CH_A_OUT_CSR_BASE;
	h2p_fifo_sink_ch_a_data_addr	= h2f_lw_axi_master + FIFO_SINK_CH_A_OUT_BASE;
	h2p_fifo_sink_ch_b_csr_addr		= h2f_lw_axi_master + FIFO_SINK_CH_B_OUT_CSR_BASE;
	h2p_fifo_sink_ch_b_data_addr	= h2f_lw_axi_master + FIFO_SINK_CH_B_OUT_BASE;
	h2p_fifo_sink_ch_c_csr_addr		= h2f_lw_axi_master + FIFO_SINK_CH_C_OUT_CSR_BASE;
	h2p_fifo_sink_ch_c_data_addr	= h2f_lw_axi_master + FIFO_SINK_CH_C_OUT_BASE;
	h2p_fifo_sink_ch_d_csr_addr		= h2f_lw_axi_master + FIFO_SINK_CH_D_OUT_CSR_BASE;
	h2p_fifo_sink_ch_d_data_addr	= h2f_lw_axi_master + FIFO_SINK_CH_D_OUT_BASE;
	h2p_fifo_sink_ch_e_csr_addr		= h2f_lw_axi_master + FIFO_SINK_CH_E_OUT_CSR_BASE;
	h2p_fifo_sink_ch_e_data_addr	= h2f_lw_axi_master + FIFO_SINK_CH_E_OUT_BASE;
	h2p_fifo_sink_ch_f_csr_addr		= h2f_lw_axi_master + FIFO_SINK_CH_F_OUT_CSR_BASE;
	h2p_fifo_sink_ch_f_data_addr	= h2f_lw_axi_master + FIFO_SINK_CH_F_OUT_BASE;
	h2p_fifo_sink_ch_g_csr_addr		= h2f_lw_axi_master + FIFO_SINK_CH_G_OUT_CSR_BASE;
	h2p_fifo_sink_ch_g_data_addr	= h2f_lw_axi_master + FIFO_SINK_CH_G_OUT_BASE;
	h2p_fifo_sink_ch_h_csr_addr		= h2f_lw_axi_master + FIFO_SINK_CH_H_OUT_CSR_BASE;
	h2p_fifo_sink_ch_h_data_addr	= h2f_lw_axi_master + FIFO_SINK_CH_H_OUT_BASE;
	h2p_led_addr					= h2f_lw_axi_master + LED_PIO_BASE;
	h2p_sw_addr						= h2f_lw_axi_master + DIPSW_PIO_BASE;
	h2p_button_addr					= h2f_lw_axi_master + BUTTON_PIO_BASE;
	h2p_adcspi_addr					= h2f_lw_axi_master + AD9276_SPI_BASE;
	h2p_adc_samples_per_echo_addr	= h2f_lw_axi_master + ADC_SAMPLES_PER_ECHO_BASE;
	h2p_init_delay_addr				= h2f_lw_axi_master + ADC_INIT_DELAY_BASE;
	//h2p_spi_num_of_bits_addr		= h2f_lw_axi_master + LM96570_SPI_NUM_OF_BITS_BASE;
	h2p_general_cnt_int_addr		= h2f_lw_axi_master + GENERAL_CNT_IN_BASE;
	h2p_general_cnt_out_addr		= h2f_lw_axi_master + GENERAL_CNT_OUT_BASE;
	//h2p_lm96570_spi_out2_addr		= h2f_lw_axi_master + LM96570_SPI_OUT_2_BASE;
	//h2p_lm96570_spi_out1_addr		= h2f_lw_axi_master + LM96570_SPI_OUT_1_BASE;
	//h2p_lm96570_spi_out0_addr		= h2f_lw_axi_master + LM96570_SPI_OUT_0_BASE;
	//h2p_lm96570_spi_in2_addr		= h2f_lw_axi_master + LM96570_SPI_IN_2_BASE;
	//h2p_lm96570_spi_in1_addr		= h2f_lw_axi_master + LM96570_SPI_IN_1_BASE;
	//h2p_lm96570_spi_in0_addr		= h2f_lw_axi_master + LM96570_SPI_IN_0_BASE;
	h2p_adc_start_pulselength_addr	= h2f_lw_axi_master + ADC_START_PULSELENGTH_BASE;
	//h2p_mux_control_addr			= h2f_lw_axi_master + MUX_CONTROL_BASE;


	// write default value for cnt_out
	cnt_out_val = CNT_OUT_DEFAULT;

	// turn on the ADC
	cnt_out_val &= ~ADC_AD9276_STBY_msk;
	cnt_out_val &= ~ADC_AD9276_PWDN_msk;
	alt_write_word( (h2p_general_cnt_out_addr) ,  cnt_out_val);
	usleep(100000);

	// turn power on
	cnt_out_val |= (POS_5V_EN_msk | NEG_5V_EN_msk);
	alt_write_word( h2p_general_cnt_out_addr ,  cnt_out_val);
	//cnt_out_val |= (NEG_48V_EN_msk | POS_48V_EN_msk);
	//alt_write_word( h2p_general_cnt_out_addr ,  cnt_out_val);

	cnt_out_val |= CLK_EN_msk;
	alt_write_word( (h2p_general_cnt_out_addr) ,  cnt_out_val);
	usleep(100000);

	/* FSM reset
	cnt_out_val |= (FSM_RESET_msk);
	alt_write_word( h2p_general_cnt_out_addr ,  cnt_out_val);
	usleep(100);
	cnt_out_val &= ~(FSM_RESET_msk);
	alt_write_word( h2p_general_cnt_out_addr ,  cnt_out_val);
	usleep(300000);
	*/

}

void leave() {
	// turn of the ADC
	cnt_out_val |= ADC_AD9276_STBY_msk;
	alt_write_word( (h2p_general_cnt_out_addr) ,  cnt_out_val);
	cnt_out_val |= ADC_AD9276_PWDN_msk;
	alt_write_word( (h2p_general_cnt_out_addr) ,  cnt_out_val);

	// turn power off
	cnt_out_val &= ~POS_5V_EN_msk;
	alt_write_word( h2p_general_cnt_out_addr ,  cnt_out_val);
	cnt_out_val &= ~NEG_5V_EN_msk;
	alt_write_word( h2p_general_cnt_out_addr ,  cnt_out_val);
	cnt_out_val &= ~NEG_48V_EN_msk;
	alt_write_word( h2p_general_cnt_out_addr ,  cnt_out_val);
	cnt_out_val &= ~POS_48V_EN_msk;
	alt_write_word( h2p_general_cnt_out_addr ,  cnt_out_val);

    close(fd_dev_mem);

    // munmap hps peripherals
    if (munmap(hps_gpio, hps_gpio_span) != 0) {
            printf("Error: hps_gpio munmap() failed\n");
            printf("    errno = %s\n", strerror(errno));
            close(fd_dev_mem);
            exit(EXIT_FAILURE);
        }

        hps_gpio = NULL;


	if (munmap(h2f_lw_axi_master, h2f_lw_axi_master_span) != 0) {
		printf("Error: h2f_lw_axi_master munmap() failed\n");
		printf("    errno = %s\n", strerror(errno));
		close(fd_dev_mem);
		exit(EXIT_FAILURE);
	}

	h2f_lw_axi_master				= NULL;
	h2p_led_addr					= NULL;
	h2p_sw_addr						= NULL;
	h2p_fifo_sink_ch_a_csr_addr		= NULL;
	h2p_fifo_sink_ch_a_data_addr	= NULL;
	h2p_fifo_sink_ch_b_csr_addr		= NULL;
	h2p_fifo_sink_ch_b_data_addr	= NULL;
	h2p_fifo_sink_ch_c_csr_addr		= NULL;
	h2p_fifo_sink_ch_d_data_addr	= NULL;
	h2p_fifo_sink_ch_d_csr_addr		= NULL;
	h2p_fifo_sink_ch_d_data_addr	= NULL;
	h2p_fifo_sink_ch_e_csr_addr		= NULL;
	h2p_fifo_sink_ch_e_data_addr	= NULL;
	h2p_fifo_sink_ch_f_csr_addr		= NULL;
	h2p_fifo_sink_ch_f_data_addr	= NULL;
	h2p_fifo_sink_ch_g_csr_addr		= NULL;
	h2p_fifo_sink_ch_g_data_addr	= NULL;
	h2p_fifo_sink_ch_h_csr_addr		= NULL;
	h2p_fifo_sink_ch_h_data_addr	= NULL;
	h2p_led_addr					= NULL;
	h2p_sw_addr						= NULL;
	h2p_button_addr					= NULL;
	h2p_adcspi_addr					= NULL;
	h2p_adc_samples_per_echo_addr	= NULL;
	h2p_init_delay_addr				= NULL;
	h2p_spi_num_of_bits_addr		= NULL;
	h2p_general_cnt_int_addr		= NULL;
	h2p_general_cnt_out_addr		= NULL;
	h2p_lm96570_spi_out2_addr		= NULL;
	h2p_lm96570_spi_out1_addr		= NULL;
	h2p_lm96570_spi_out0_addr		= NULL;
	h2p_lm96570_spi_in2_addr		= NULL;
	h2p_lm96570_spi_in1_addr		= NULL;
	h2p_lm96570_spi_in0_addr		= NULL;
	h2p_mux_control_addr			= NULL;
	
	printf("\nULTRASOUND SYSTEM STOPS!\n");
}

unsigned int write_adc_spi (unsigned int comm) {
	unsigned int data;

	while (! (alt_read_word(h2p_adcspi_addr + SPI_STATUS_offst) & (1<<status_TRDY_bit)));
	alt_write_word( (h2p_adcspi_addr + SPI_TXDATA_offst) ,  comm);
	while (! (alt_read_word(h2p_adcspi_addr + SPI_STATUS_offst) & (1<<status_TMT_bit)));
	data = alt_read_word(h2p_adcspi_addr + SPI_RXDATA_offst); // wait for the spi command to finish
	return (data);
}

unsigned int write_ad9276_spi (unsigned char rw, unsigned int addr, unsigned int val) {
	unsigned int command, data;
	unsigned int comm;

	comm = (rw<<7) | AD9276_1BYTE_DATA;	// set command to write a byte data
	if (rw == AD9276_SPI_RD) val = 0x00;

	command = (comm<<16) | (addr<<8) | (val); 		//
	data = write_adc_spi (command);
	printf("command = 0x%06x -> datain = 0x%06x\n", command, data);
	return data;
}

void write_beamformer_spi (unsigned char spi_reg_length, unsigned char read, unsigned char spi_addr, unsigned long spi_data_out, unsigned int *spi_in0, unsigned int *spi_in1, unsigned int *spi_in2) {
	unsigned int spi_out0, spi_out1, spi_out2;
	spi_out0 = 0;
	spi_out1 = 0;
	spi_out2 = 0;

	spi_out0 = (spi_addr & 0x1F) | ((read & 0x01)<<5) | ( (spi_data_out & 0x3FFFFFF)<<6);
	spi_out1 = (spi_data_out>>26) & 0xFFFFFFFF;
	//spi_out2 = (unsigned int)(spi_data_out>>58) & (unsigned int)0x3F; WARNING!

	alt_write_word( h2p_lm96570_spi_in0_addr ,  spi_out0);
	alt_write_word( h2p_lm96570_spi_in1_addr ,  spi_out1);
	alt_write_word( h2p_lm96570_spi_in2_addr ,  spi_out2);
	alt_write_word( h2p_spi_num_of_bits_addr ,  spi_reg_length + 6); // 6 is the command length

	cnt_out_val |= BF_SPI_START_msk;
	alt_write_word( h2p_general_cnt_out_addr ,  cnt_out_val); // start the beamformer SPI
	usleep(10);
	cnt_out_val &= (~BF_SPI_START_msk);
	alt_write_word( h2p_general_cnt_out_addr ,  cnt_out_val); // stop the beamformer SPI

	*spi_in0 = alt_read_word(h2p_lm96570_spi_out0_addr);
	*spi_in1 = alt_read_word(h2p_lm96570_spi_out1_addr);
	*spi_in2 = alt_read_word(h2p_lm96570_spi_out2_addr);

	printf("beamformer_spi_in = 0x%04x_%04x_%04x\n", *spi_in2, *spi_in1, *spi_in0);

}

void read_adc_id () {
	unsigned int command, data;
	unsigned int comm, addr;

	comm = (1<<7) | (0<<6) | (0<<5); // read chip settings
	addr = 0x00;
	command = (comm<<16) | (addr<<8) | 0x00;
	data = write_adc_spi (command);
	printf("command = 0x%06x -> datain = 0x%06x\n", command, data);

	comm = (1<<7) | (0<<6) | (0<<5); // read chip ID
	addr = 0x01;
	command = (comm<<16) | (addr<<8) | 0x00;
	data = write_adc_spi (command);
	printf("command = 0x%06x -> datain = 0x%06x\n", command, data);

	comm = (1<<7) | (0<<6) | (0<<5); // read chip ID
	addr = 0x02;
	command = (comm<<16) | (addr<<8) | 0x00;
	data = write_adc_spi (command);
	printf("command = 0x%06x -> datain = 0x%06x\n", command, data);

}

void init_adc() {
	write_ad9276_spi (AD9276_SPI_WR, AD9276_CHIP_PORT_CONF_REG, 0b00111100); // reset
	write_ad9276_spi (AD9276_SPI_WR, AD9276_DEV_UPDT_REG, AD9276_SW_TRF_MSK); // update the device
	write_ad9276_spi (AD9276_SPI_WR, AD9276_CHIP_PORT_CONF_REG, 0b00011000); // reset

	write_ad9276_spi (AD9276_SPI_WR, AD9276_FLEX_GAIN_REG, AD9276_PGA_GAIN_21dB_VAL<<AD9276_PGA_GAIN_SHFT | AD9276_LNA_GAIN_15dB_VAL<<AD9276_LNA_GAIN_SHFT); // set PGA Gain to 21 dB, LNA Gain to 15.6 dB
	write_ad9276_spi (AD9276_SPI_WR, AD9276_OUT_ADJ_REG, AD9276_OUT_ADJ_TERM_200OHM_VAL<<AD9276_OUT_ADJ_TERM_SHFT); // set output driver to 100 ohms
	write_ad9276_spi (AD9276_SPI_WR, AD9276_OUT_PHS_REG, AD9276_OUT_PHS_000DEG_VAL); // set phase to 000 degrees
	write_ad9276_spi (AD9276_SPI_WR, AD9276_DEV_UPDT_REG, AD9276_SW_TRF_MSK); // update the device

	// filter setup
	write_ad9276_spi (AD9276_SPI_WR, AD9276_FLEX_FILT_REG, AD9276_FLEX_FILT_HPF_04PCTG_FLP_VAL); // set high-pass filter
	write_ad9276_spi (AD9276_SPI_WR, AD9276_DEV_UPDT_REG, AD9276_SW_TRF_MSK); // update the device

	write_ad9276_spi (AD9276_SPI_WR, AD9276_DEV_IDX_1_REG, AD9276_DCO_CMD_EN_MSK| AD9276_FCO_CMD_EN_MSK); // select DCO and FCO
	write_ad9276_spi (AD9276_SPI_WR, AD9276_DEV_IDX_2_REG, 0x00);
	write_ad9276_spi (AD9276_SPI_WR, AD9276_OUT_MODE_REG, AD9276_OUT_MODE_INVERT_EN_MSK); // invert DCO and FCO
	write_ad9276_spi (AD9276_SPI_WR, AD9276_DEV_UPDT_REG, AD9276_SW_TRF_MSK); // update the device

	write_ad9276_spi (AD9276_SPI_WR, AD9276_DEV_IDX_1_REG, AD9276_CH_A_CMD_EN_MSK|AD9276_CH_B_CMD_EN_MSK|AD9276_CH_C_CMD_EN_MSK|AD9276_CH_D_CMD_EN_MSK); // select channel A-D
	write_ad9276_spi (AD9276_SPI_WR, AD9276_DEV_IDX_2_REG, AD9276_CH_E_CMD_EN_MSK|AD9276_CH_F_CMD_EN_MSK|AD9276_CH_G_CMD_EN_MSK|AD9276_CH_H_CMD_EN_MSK); // select channel E-H
	write_ad9276_spi (AD9276_SPI_WR, AD9276_OUT_MODE_REG, AD9276_OUT_MODE_INVERT_EN_MSK); // invert all selected channel
	write_ad9276_spi (AD9276_SPI_WR, AD9276_TESTIO_REG, 0x00); // disable test I/O

	// write pattern (comment these 2 lines to disable)
	// write_ad9276_spi (AD9276_SPI_WR, AD9276_TESTIO_REG, AD9276_OUT_TEST_PNSEQ_LONG_VAL); // select testpattern
	// write_ad9276_spi (AD9276_SPI_WR, AD9276_DEV_UPDT_REG, AD9276_SW_TRF_MSK); // update the device

	//write_ad9276_spi (AD9276_SPI_RD, AD9276_OUT_ADJ_REG, 0x00);		// check output driver termination of selected channel
	//write_ad9276_spi (AD9276_SPI_RD, AD9276_FLEX_GAIN_REG, 0x00);		// check flex_gain of selected channel
	//write_ad9276_spi (AD9276_SPI_RD, AD9276_OUT_PHS_REG, 0x00);		// check output_phase

	usleep(10000);
}

void adc_wr_testval (uint16_t val1, uint16_t val2) { // write test value for the ADC output
	write_ad9276_spi (AD9276_SPI_WR, AD9276_TESTIO_REG, AD9276_OUT_TEST_USR_INPUT_VAL); // user input test

	write_ad9276_spi (AD9276_SPI_WR, AD9276_USR_PATT1_LSB_REG, (uint8_t)(val1& 0xFF)); // user input values
	write_ad9276_spi (AD9276_SPI_WR, AD9276_USR_PATT1_MSB_REG, (uint8_t)((val1>>8) & 0xFF)); // user input values

	write_ad9276_spi (AD9276_SPI_WR, AD9276_USR_PATT2_LSB_REG, (uint8_t)(val2 & 0xFF)); // user input values
	write_ad9276_spi (AD9276_SPI_WR, AD9276_USR_PATT2_MSB_REG, (uint8_t)((val2>>8) & 0xFF)); // user input values

	write_ad9276_spi (AD9276_SPI_WR, AD9276_DEV_UPDT_REG, AD9276_SW_TRF_MSK); // update the device
	usleep(10000);
}

void init_beamformer() {
	unsigned int spi_in0, spi_in1, spi_in2;
	spi_in0 = 0x00;
	spi_in1 = 0x00;
	spi_in2 = 0x00;

	write_beamformer_spi(REG_1A_LENGTH, 	LM86570_SPI_WR, 0x1A, 0x4600, &spi_in0, &spi_in1, &spi_in2);

	write_beamformer_spi(REG_00_07_LENGTH, 	LM86570_SPI_WR, 0x00, 0x0000, &spi_in0, &spi_in1, &spi_in2);
	write_beamformer_spi(REG_00_07_LENGTH, 	LM86570_SPI_WR, 0x01, 0x0000, &spi_in0, &spi_in1, &spi_in2);
	write_beamformer_spi(REG_00_07_LENGTH, 	LM86570_SPI_WR, 0x02, 0x0000, &spi_in0, &spi_in1, &spi_in2);
	write_beamformer_spi(REG_00_07_LENGTH, 	LM86570_SPI_WR, 0x03, 0x0000, &spi_in0, &spi_in1, &spi_in2);
	write_beamformer_spi(REG_00_07_LENGTH, 	LM86570_SPI_WR, 0x04, 0x0000, &spi_in0, &spi_in1, &spi_in2);
	write_beamformer_spi(REG_00_07_LENGTH, 	LM86570_SPI_WR, 0x05, 0x0000, &spi_in0, &spi_in1, &spi_in2);
	write_beamformer_spi(REG_00_07_LENGTH, 	LM86570_SPI_WR, 0x06, 0x0000, &spi_in0, &spi_in1, &spi_in2);
	write_beamformer_spi(REG_00_07_LENGTH, 	LM86570_SPI_WR, 0x07, 0x0000, &spi_in0, &spi_in1, &spi_in2);

	/*
	write_beamformer_spi(4, 				LM86570_SPI_WR, 0x08, 0x0005, &spi_in0, &spi_in1, &spi_in2);
	write_beamformer_spi(4, 				LM86570_SPI_WR, 0x09, 0x0005, &spi_in0, &spi_in1, &spi_in2);
	write_beamformer_spi(4, 				LM86570_SPI_WR, 0x0A, 0x0005, &spi_in0, &spi_in1, &spi_in2);
	write_beamformer_spi(4, 				LM86570_SPI_WR, 0x0B, 0x0005, &spi_in0, &spi_in1, &spi_in2);
	write_beamformer_spi(4, 				LM86570_SPI_WR, 0x0C, 0x0005, &spi_in0, &spi_in1, &spi_in2);
	write_beamformer_spi(4, 				LM86570_SPI_WR, 0x0D, 0x0005, &spi_in0, &spi_in1, &spi_in2);
	write_beamformer_spi(4, 				LM86570_SPI_WR, 0x0E, 0x0005, &spi_in0, &spi_in1, &spi_in2);
	write_beamformer_spi(4, 				LM86570_SPI_WR, 0x0F, 0x0005, &spi_in0, &spi_in1, &spi_in2);

	write_beamformer_spi(4, 				LM86570_SPI_WR, 0x10, 0x000A, &spi_in0, &spi_in1, &spi_in2);
	write_beamformer_spi(4, 				LM86570_SPI_WR, 0x11, 0x000A, &spi_in0, &spi_in1, &spi_in2);
	write_beamformer_spi(4, 				LM86570_SPI_WR, 0x12, 0x000A, &spi_in0, &spi_in1, &spi_in2);
	write_beamformer_spi(4, 				LM86570_SPI_WR, 0x13, 0x000A, &spi_in0, &spi_in1, &spi_in2);
	write_beamformer_spi(4, 				LM86570_SPI_WR, 0x14, 0x000A, &spi_in0, &spi_in1, &spi_in2);
	write_beamformer_spi(4, 				LM86570_SPI_WR, 0x15, 0x000A, &spi_in0, &spi_in1, &spi_in2);
	write_beamformer_spi(4, 				LM86570_SPI_WR, 0x16, 0x000A, &spi_in0, &spi_in1, &spi_in2);
	write_beamformer_spi(4, 				LM86570_SPI_WR, 0x17, 0x000A, &spi_in0, &spi_in1, &spi_in2);
	*/

	write_beamformer_spi(4, 				LM86570_SPI_WR, 0x18, 0x0005, &spi_in0, &spi_in1, &spi_in2); // write all p registers
	write_beamformer_spi(4, 				LM86570_SPI_WR, 0x19, 0x000A, &spi_in0, &spi_in1, &spi_in2); // write all n registers

	write_beamformer_spi(4, 				REG_1BA_LENGTH, 0x1B, 0x0000, &spi_in0, &spi_in1, &spi_in2);

}

void read_adc_val (void *channel_csr_addr, void *channel_data_addr, unsigned int * adc_data) {//, char *filename) {
	unsigned int fifo_mem_level;
	//fptr = fopen(filename, "w");
	//if (fptr == NULL) {
	//	printf("File does not exists \n");
	//	return;
	//}

	// PRINT # of DATAS in FIFO
	fifo_mem_level = alt_read_word(channel_csr_addr+ALTERA_AVALON_FIFO_LEVEL_REG); // the fill level of FIFO memory
	printf("fifo data before reading: %d ---",fifo_mem_level);
	//

	// READING DATA FROM FIFO
	fifo_mem_level = alt_read_word(channel_csr_addr+ALTERA_AVALON_FIFO_LEVEL_REG); // the fill level of FIFO memory
	for (i=0; fifo_mem_level>0; i++) {
		adc_data[i] = alt_read_word(channel_data_addr);

		//fprintf(fptr, "%d\n", rddata[i] & 0xFFF);
		//fprintf(fptr, "%d\n", (rddata[i]>>16) & 0xFFF);

		fifo_mem_level--;
		if (fifo_mem_level == 0) {
			fifo_mem_level = alt_read_word(channel_csr_addr+ALTERA_AVALON_FIFO_LEVEL_REG);
		}
	}
	usleep(100);

	fifo_mem_level = alt_read_word(channel_csr_addr+ALTERA_AVALON_FIFO_LEVEL_REG); // the fill level of FIFO memory
	printf("fifo data after reading: %d\n",fifo_mem_level);

	//fclose(fptr);

}


void store_data_2d (unsigned int * adc_data, unsigned int data_bank_2d[num_of_channels][num_of_samples], unsigned int ch_num, unsigned int num_of_samples) {
	unsigned int ii;
	for (ii=0; ii<num_of_samples/2; ii++) {
		data_bank_2d[ch_num][ii*2] = adc_data[ii] & 0xFFF;
		data_bank_2d[ch_num][ii*2+1] = (adc_data[ii]>>16) & 0xFFF;
	}
}


void write_data_bank (unsigned int data_bank[num_of_switches][num_of_channels][num_of_samples]) {
	unsigned int ii, jj, kk;

	fptr = fopen("databank.txt", "w");
		if (fptr == NULL) {
		printf("File does not exists \n");
		return;
	}

	for (ii=0; ii<num_of_switches; ii++) {
		for (jj=0; jj<num_of_channels; jj++) {
			for (kk=0; kk<num_of_samples; kk++) {
				fprintf(fptr, "%d,", data_bank[ii][jj][kk] & 0xFFF);
			}
			fprintf(fptr, "\n");
		}
		fprintf(fptr, "\n");
	}
}

void write_data_2d (unsigned int data_bank_2d[num_of_channels][num_of_samples]) {
	unsigned int jj, kk;

	fptr = fopen("databank.txt", "w");
		if (fptr == NULL) {
		printf("File does not exists \n");
		return;
	}

	for (jj=0; jj<num_of_channels; jj++) {
		for (kk=0; kk<num_of_samples; kk++) {
			fprintf(fptr, "%d", data_bank_2d[jj][kk] & 0xFFF);
			if (kk!= num_of_samples-1) { // this is to avoid generating COMMA at the end of the file
				fprintf(fptr, ",");
			}
		}
		fprintf(fptr, "\n");
	}

}

int main (int argc, char * argv[]){

	uint16_t val1 = atoi(argv[1]);
	uint16_t val2 = atoi(argv[2]);

	// Initialize system
    init();

    // init_beamformer();

	read_adc_id();
    init_adc();
    // adc_wr_testval (val1, val2);

    alt_write_word( h2p_adc_start_pulselength_addr , 10 );
    alt_write_word( h2p_adc_samples_per_echo_addr ,  num_of_samples);

    unsigned int data_bank_2d[num_of_channels][num_of_samples];
    unsigned int adc_data [num_of_samples]; // data for 1 acquisition
    char sw_num = 0;

    usleep(200000);

	// tx_enable fire
	cnt_out_val |= BF_TX_EN_msk;
	alt_write_word( h2p_general_cnt_out_addr ,  cnt_out_val); // start the beamformer SPI
	usleep(10000);
	cnt_out_val &= (~BF_TX_EN_msk);
	alt_write_word( h2p_general_cnt_out_addr ,  cnt_out_val); // stop the beamformer SPI
	usleep(10000);

	// pulser off
	//usleep(200000);
	//cnt_out_val &= (~PULSER_EN_msk);
	//alt_write_word( h2p_general_cnt_out_addr ,  cnt_out_val); // stop the beamformer SPI

	read_adc_val(h2p_fifo_sink_ch_a_csr_addr, h2p_fifo_sink_ch_a_data_addr, adc_data);
	store_data_2d (adc_data, data_bank_2d, 0, num_of_samples);

	read_adc_val(h2p_fifo_sink_ch_b_csr_addr, h2p_fifo_sink_ch_b_data_addr, adc_data);
	store_data_2d (adc_data, data_bank_2d, 1, num_of_samples);

	read_adc_val(h2p_fifo_sink_ch_c_csr_addr, h2p_fifo_sink_ch_c_data_addr, adc_data);
	store_data_2d (adc_data, data_bank_2d, 2, num_of_samples);

	read_adc_val(h2p_fifo_sink_ch_d_csr_addr, h2p_fifo_sink_ch_d_data_addr, adc_data);
	store_data_2d (adc_data, data_bank_2d, 3, num_of_samples);

	read_adc_val(h2p_fifo_sink_ch_e_csr_addr, h2p_fifo_sink_ch_e_data_addr, adc_data);
	store_data_2d (adc_data, data_bank_2d, 4, num_of_samples);

	read_adc_val(h2p_fifo_sink_ch_f_csr_addr, h2p_fifo_sink_ch_f_data_addr, adc_data);
	store_data_2d (adc_data, data_bank_2d, 5, num_of_samples);

	read_adc_val(h2p_fifo_sink_ch_g_csr_addr, h2p_fifo_sink_ch_g_data_addr, adc_data);
	store_data_2d (adc_data, data_bank_2d, 6, num_of_samples);

	read_adc_val(h2p_fifo_sink_ch_h_csr_addr, h2p_fifo_sink_ch_h_data_addr, adc_data);
	store_data_2d (adc_data, data_bank_2d, 7, num_of_samples);

	printf("Completed Event: %d\n",sw_num);

	write_data_2d (data_bank_2d);



    // exit program
    leave();

    return 0;
}
