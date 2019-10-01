/* offsets for output control signal
#define ADC_AD9276_PWDN_OFST				(9)
#define ADC_AD9276_STBY_OFST				(8)
#define FSM_RESET_OFST					(7)
#define sw_off_OFST					(6)
#define tx_path_en_OFST				(5)
#define pulser_en_OFST				(4)
#define lm96570_pin_reset_OFST		(3)
#define lm96570_tx_en_OFST			(2)
#define lm96570_spi_reset_OFST		(1)
#define lm96570_start_OFST			(0)

// mask for output control signal
#define ADC_AD9276_PWDN_MSK			(1<<ADC_AD9276_PWDN_OFST			)
#define ADC_AD9276_STBY_MSK			(1<<ADC_AD9276_STBY_OFST			)
#define FSM_RESET_MSK				(1<<FSM_RESET_OFST				)
#define sw_off_MSK					(1<<sw_off_OFST				)
#define tx_path_en_MSK				(1<<tx_path_en_OFST			)
#define pulser_en_MSK				(1<<pulser_en_OFST			)
#define lm96570_pin_reset_MSK		(1<<lm96570_pin_reset_OFST	)
#define lm96570_tx_en_MSK			(1<<lm96570_tx_en_OFST		)
#define lm96570_spi_reset_MSK		(1<<lm96570_spi_reset_OFST	)
#define lm96570_start_MSK			(1<<lm96570_start_OFST		)
*/

// OUTPUT
#define CLK_EN_ofst					(21)
#define NEG_5V_EN_ofst		    	(20)
#define POS_48V_EN_ofst             (19)
#define NEG_48V_EN_ofst             (18)
#define POS_5V_EN_ofst              (17)
#define MAX14808_MODE1_ofst         (16)
#define MAX14808_MODE0_ofst         (15)
#define MAX14808_CC1_ofst           (14)
#define MAX14808_CC0_ofst           (13)
#define LTC2640_CLR_n_ofst          (12)
#define ADC_AD9276_4LORESET_ofst    (11)
#define TX_OE_ofst				    (10)
#define ADC_AD9276_PWDN_ofst         (9)
#define ADC_AD9276_STBY_ofst         (8)
#define FSM_RESET_ofst               (7)
#define TXRX_SW_OFF_ofst             (6)
#define TXRX_SPI_EN_ofst             (5)
#define PULSER_EN_ofst               (4)
#define BF_RESET_ofst                (3)
#define BF_TX_EN_ofst                (2)
#define BF_SPI_RESET_ofst            (1)
#define BF_SPI_START_ofst            (0)

#define CLK_EN_msk					 (1<<CLK_EN_ofst		      )
#define NEG_5V_EN_msk		         (1<<NEG_5V_EN_ofst		      )
#define POS_48V_EN_msk               (1<<POS_48V_EN_ofst          )
#define NEG_48V_EN_msk               (1<<NEG_48V_EN_ofst          )
#define POS_5V_EN_msk                (1<<POS_5V_EN_ofst           )
#define MAX14808_MODE1_msk           (1<<MAX14808_MODE1_ofst      )
#define MAX14808_MODE0_msk           (1<<MAX14808_MODE0_ofst      )
#define MAX14808_CC1_msk             (1<<MAX14808_CC1_ofst        )
#define MAX14808_CC0_msk             (1<<MAX14808_CC0_ofst        )
#define LTC2640_CLR_n_msk            (1<<LTC2640_CLR_n_ofst       )
#define ADC_AD9276_4LORESET_msk      (1<<ADC_AD9276_4LORESET_ofst )
#define TX_OE_msk				     (1<<TX_OE_ofst				  )
#define ADC_AD9276_PWDN_msk          (1<<ADC_AD9276_PWDN_ofst     )
#define ADC_AD9276_STBY_msk          (1<<ADC_AD9276_STBY_ofst     )
#define FSM_RESET_msk                (1<<FSM_RESET_ofst           )
#define TXRX_SW_OFF_msk              (1<<TXRX_SW_OFF_ofst         )
#define TXRX_SPI_EN_msk              (1<<TXRX_SPI_EN_ofst         )
#define PULSER_EN_msk                (1<<PULSER_EN_ofst           )
#define BF_RESET_msk                 (1<<BF_RESET_ofst            )
#define BF_TX_EN_msk                 (1<<BF_TX_EN_ofst            )
#define BF_SPI_RESET_msk             (1<<BF_SPI_RESET_ofst        )
#define BF_SPI_START_msk             (1<<BF_SPI_START_ofst        )


// INPUT
#define MAX_14808_SYNC_ofst    (3)
#define MAX14808_THP_ofst      (2)
#define FSM_DONE_ofst          (1)
#define BF_SPI_DONE_ofst       (0)

#define MAX_14808_SYNC_msk     (1<<MAX_14808_SYNC_ofst  )
#define MAX14808_THP_msk       (1<<MAX14808_THP_ofst    )
#define FSM_DONE_msk           (1<<FSM_DONE_ofst        )
#define BF_SPI_DONE_msk        (1<<BF_SPI_DONE_ofst     )










// default for output control signal
#define CNT_OUT_DEFAULT 0b000000
