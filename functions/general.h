/* OUTPUT CONTROL SIGNAL TO FPGA */
#define CNT_OUT_default		0x00

// offsets for output control signal
#define nmr_controller_reset_ofst	(5)
#define adc_fifo_rst_ofst			(4)
#define phase_cycle_ofst			(3)
#define pll_adc_rst_ofst			(2)
#define pll_nmr_rst_ofst			(1)
#define nmr_controller_start_ofst	(0)
// output mask signal for FPGA
#define nmr_controller_reset	(1<<nmr_controller_reset_ofst)
#define adc_fifo_rst			(1<<adc_fifo_rst_ofst)
#define phase_cycle				(1<<phase_cycle_ofst)
#define pll_adc_rst				(1<<pll_adc_rst_ofst)
#define pll_nmr_rst				(1<<pll_nmr_rst_ofst)
#define nmr_controller_start	(1<<nmr_controller_start_ofst)


/* INPUT CONTROL SIGNAL FROM FPGA */
// Offsets for input status signal
#define adc_fifo_in_ready_ofst			(3)
#define pll_adc_lock_ofst				(2)
#define pll_nmr_lock_ofst				(1)
#define NMR_controller_systemstat_ofst	(0)
// Input mask signal from FPGA
#define adc_fifo_in_ready				(1<<adc_fifo_in_ready_ofst)
#define pll_adc_lock					(1<<pll_adc_lock_ofst)
#define pll_nmr_lock					(1<<pll_nmr_lock_ofst)
#define NMR_controller_systemstat		(1<<NMR_controller_systemstat_ofst)



// general variable
#define ENABLE_MESSAGE	1
#define DISABLE_MESSAGE 0
#define ENABLE 1
#define DISABLE 0

