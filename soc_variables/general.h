// offsets for output control signal
#define ADC_AD9276_PWDN				(9)
#define ADC_AD9276_STBY				(8)
#define FSM_RESET					(7)
#define sw_off_OFST					(6)
#define tx_path_en_OFST				(5)
#define pulser_en_OFST				(4)
#define lm96570_pin_reset_OFST		(3)
#define lm96570_tx_en_OFST			(2)
#define lm96570_spi_reset_OFST		(1)
#define lm96570_start_OFST			(0)

// mask for output control signal
#define ADC_AD9276_PWDN_MSK			(1<<ADC_AD9276_PWDN			)
#define ADC_AD9276_STBY_MSK			(1<<ADC_AD9276_STBY			)
#define FSM_RESET_MSK				(1<<FSM_RESET				)
#define sw_off_MSK					(1<<sw_off_OFST				)
#define tx_path_en_MSK				(1<<tx_path_en_OFST			)
#define pulser_en_MSK				(1<<pulser_en_OFST			)
#define lm96570_pin_reset_MSK		(1<<lm96570_pin_reset_OFST	)
#define lm96570_tx_en_MSK			(1<<lm96570_tx_en_OFST		)
#define lm96570_spi_reset_MSK		(1<<lm96570_spi_reset_OFST	)
#define lm96570_start_MSK			(1<<lm96570_start_OFST		)

// default for output control signal
#define CNT_OUT_DEFAULT 0b000000
