#ifndef HPS_LINUX_H_
#define HPS_LINUX_H_

#include <stdbool.h>
#include <stdint.h>

#include "socal/hps.h"
#include "./soc_variables/soc_system.h"

// |=============|==========|==============|==========|
// | Signal Name | HPS GPIO | Register/bit | Function |
// |=============|==========|==============|==========|
// |   HPS_LED   |  GPIO53  |   GPIO1[24]  |    I/O   |
// |=============|==========|==============|==========|
#define HPS_LED_IDX      (ALT_GPIO_1BIT_53)                      // GPIO53
#define HPS_LED_PORT     (alt_gpio_bit_to_pid(HPS_LED_IDX))      // ALT_GPIO_PORTB
#define HPS_LED_PORT_BIT (alt_gpio_bit_to_port_pin(HPS_LED_IDX)) // 24 (from GPIO1[24])
#define HPS_LED_MASK     (1 << HPS_LED_PORT_BIT)

// |=============|==========|==============|==========|
// | Signal Name | HPS GPIO | Register/bit | Function |
// |=============|==========|==============|==========|
// |  HPS_KEY_N  |  GPIO54  |   GPIO1[25]  |    I/O   |
// |=============|==========|==============|==========|
#define HPS_KEY_N_IDX      (ALT_GPIO_1BIT_54)                        // GPIO54
#define HPS_KEY_N_PORT     (alt_gpio_bit_to_pid(HPS_KEY_N_IDX))      // ALT_GPIO_PORTB
#define HPS_KEY_N_PORT_BIT (alt_gpio_bit_to_port_pin(HPS_KEY_N_IDX)) // 25 (from GPIO1[25])
#define HPS_KEY_N_MASK     (1 << HPS_KEY_N_PORT_BIT)

// physical memory file descriptor
int fd_dev_mem = 0;

// memory-mapped peripherals
void   *hps_gpio     = NULL;
size_t hps_gpio_span = ALT_GPIO1_UB_ADDR - ALT_GPIO1_LB_ADDR + 1;
size_t hps_gpio_ofst = ALT_GPIO1_OFST;

void   *h2f_lw_axi_master     = NULL;
size_t h2f_lw_axi_master_span = ALT_LWFPGASLVS_UB_ADDR - ALT_LWFPGASLVS_LB_ADDR + 1;
size_t h2f_lw_axi_master_ofst = ALT_LWFPGASLVS_OFST;

void *h2p_fifo_sink_ch_a_data_addr = NULL;
void *h2p_fifo_sink_ch_b_data_addr = NULL;
void *h2p_fifo_sink_ch_c_data_addr = NULL;
void *h2p_fifo_sink_ch_d_data_addr = NULL;
void *h2p_fifo_sink_ch_e_data_addr = NULL;
void *h2p_fifo_sink_ch_f_data_addr = NULL;
void *h2p_fifo_sink_ch_g_data_addr = NULL;
void *h2p_fifo_sink_ch_h_data_addr = NULL;

void *h2p_led_addr= NULL;
void *h2p_sw_addr= NULL;
void *h2p_button_addr= NULL;


// void *h2p_adcspi_addr = NULL;

volatile unsigned int *h2p_adcspi_addr 					= NULL; // gpio for dac (spi)

volatile unsigned int *h2p_fifo_sink_ch_a_csr_addr			= NULL; // ADC streaming FIFO status address
volatile unsigned int *h2p_fifo_sink_ch_b_csr_addr			= NULL; // ADC streaming FIFO status address
volatile unsigned int *h2p_fifo_sink_ch_c_csr_addr			= NULL; // ADC streaming FIFO status address
volatile unsigned int *h2p_fifo_sink_ch_d_csr_addr			= NULL; // ADC streaming FIFO status address
volatile unsigned int *h2p_fifo_sink_ch_e_csr_addr			= NULL; // ADC streaming FIFO status address
volatile unsigned int *h2p_fifo_sink_ch_f_csr_addr			= NULL; // ADC streaming FIFO status address
volatile unsigned int *h2p_fifo_sink_ch_g_csr_addr			= NULL; // ADC streaming FIFO status address
volatile unsigned int *h2p_fifo_sink_ch_h_csr_addr			= NULL; // ADC streaming FIFO status address

volatile unsigned int *h2p_adc_samples_per_echo_addr	= NULL;
volatile unsigned int *h2p_init_delay_addr				= NULL;
volatile unsigned int *h2p_spi_num_of_bits_addr			= NULL;
volatile unsigned int *h2p_general_cnt_int_addr			= NULL;
volatile unsigned int *h2p_general_cnt_out_addr			= NULL;

volatile unsigned int *h2p_lm96570_spi_out2_addr		= NULL;
volatile unsigned int *h2p_lm96570_spi_out1_addr		= NULL;
volatile unsigned int *h2p_lm96570_spi_out0_addr		= NULL;

volatile unsigned int *h2p_lm96570_spi_in2_addr			= NULL;
volatile unsigned int *h2p_lm96570_spi_in1_addr			= NULL;
volatile unsigned int *h2p_lm96570_spi_in0_addr			= NULL;

volatile unsigned int *h2p_adc_start_pulselength_addr 	= NULL;

volatile unsigned int *h2p_mux_control_addr 	= NULL;


void open_physical_memory_device();
void close_physical_memory_device();
void mmap_hps_peripherals();
void munmap_hps_peripherals();
void mmap_fpga_peripherals();
void munmap_fpga_peripherals();
void mmap_peripherals();
void munmap_peripherals();
void setup_hps_gpio();
void setup_fpga_leds();
void handle_hps_led();
void handle_fpga_leds();
void write_beamformer_spi (unsigned char spi_reg_length, unsigned char read, unsigned char spi_addr, unsigned long spi_data_out, unsigned int *spi_in0, unsigned int *spi_in1, unsigned int *spi_in2);

// FUNCTIONS
void create_measurement_folder();								// create a folder in the system for the measurement data
void leave();												// terminate the program
void init();													// initialize the system with tuned default parameter


// global variables
FILE	*fptr;
long i;
long j;
unsigned int rddata [10000];
unsigned int rddata_16[10000];
char foldername[50]; // variable to store folder name of the measurement data
char pathname[60];

#endif
