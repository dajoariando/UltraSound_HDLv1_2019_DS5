// (global) means for everything in the chip.
// (local) means for selected channel or selected devices, e.g. channel A-H, DCO settings, FCO settings, etc

#define AD9276_SPI_RD 1
#define AD9276_SPI_WR 0

#define AD9276_1BYTE_DATA (0x00<<5)
#define AD9276_2BYTE_DATA (0x01<<5)
#define AD9276_3BYTE_DATA (0x02<<5)

// AD9276 Memory Map Registers
	// Chip Configuration Registers
	#define AD9276_CHIP_PORT_CONF_REG		(0x00) // default 0x18, Nibbles should be mirrored so that LSB or MSB first mode is set correctly regardless of shift mode. 
	#define AD9276_CHIP_ID_REG				(0x01) // default 0x72 for AD9276
	#define AD9276_CHIP_GRADE_REG			(0x02) // default 0x00 for 40MSPS, speed power modes
	
	// Device Index and Transfer Registers
	#define AD9276_DEV_IDX_2_REG            (0x04) // default 0x0F, bits are set to determine which on-chip device receives the next write command
		#define AD9276_CH_H_CMD_EN_SHFT			(3)
		#define AD9276_CH_H_CMD_EN_MSK			(0x01<<AD9276_CH_H_CMD_EN_SHFT)
		#define AD9276_CH_G_CMD_EN_SHFT			(2)
		#define AD9276_CH_G_CMD_EN_MSK			(0x01<<AD9276_CH_G_CMD_EN_SHFT)
		#define AD9276_CH_F_CMD_EN_SHFT			(1)
		#define AD9276_CH_F_CMD_EN_MSK			(0x01<<AD9276_CH_F_CMD_EN_SHFT)
		#define AD9276_CH_E_CMD_EN_SHFT			(0)
		#define AD9276_CH_E_CMD_EN_MSK			(0x01<<AD9276_CH_E_CMD_EN_SHFT)
		
	#define AD9276_DEV_IDX_1_REG            (0x05) // default 0x0F, bits are set to determine which on-chip device receives the next write command
		#define AD9276_DCO_CMD_EN_SHFT			(5)
		#define AD9276_DCO_CMD_EN_MSK			(0x01<<AD9276_DCO_CMD_EN_SHFT)
		#define AD9276_FCO_CMD_EN_SHFT			(4)
		#define AD9276_FCO_CMD_EN_MSK			(0x01<<AD9276_FCO_CMD_EN_SHFT)
		#define AD9276_CH_D_CMD_EN_SHFT			(3)
		#define AD9276_CH_D_CMD_EN_MSK			(0x01<<AD9276_CH_D_CMD_EN_SHFT)
		#define AD9276_CH_C_CMD_EN_SHFT			(2)
		#define AD9276_CH_C_CMD_EN_MSK			(0x01<<AD9276_CH_C_CMD_EN_SHFT)
		#define AD9276_CH_B_CMD_EN_SHFT			(1)
		#define AD9276_CH_B_CMD_EN_MSK			(0x01<<AD9276_CH_B_CMD_EN_SHFT)
		#define AD9276_CH_A_CMD_EN_SHFT			(0)
		#define AD9276_CH_A_CMD_EN_MSK			(0x01<<AD9276_CH_A_CMD_EN_SHFT)
		
	#define AD9276_DEV_UPDT_REG             (0xFF) // default 0x00, synchronously transfers data from the master shift register to the slave
		#define AD9276_SW_TRF_SHFT					(0)
		#define AD9276_SW_TRF_MSK					(1<<AD9276_SW_TRF_SHFT)


	// Program Function Registers
	#define AD9276_MODES_REG                (0x08) // default 0x00, determines generic modes of chip operation (global)
		#define AD9276_LNA_Z_SHFT				(4)
		#define AD9276_LNA_Z_MSK				(0x01<<AD9276_LNA_Z_SHFT)
			#define AD9276_LNA_Z_5KOHM_VAL			(0x01)
			#define AD9276_LNA_Z_15KOHM_VAL			(0x00)
		#define AD9276_PWRDOWN_MODE_SHFT		(0)
		#define AD9276_PWRDOWN_MODE_MSK			(0b111<<AD9276_PWRDOWN_MODE_SHFT)
			#define AD9276_CHIPRUN_VAL				(0b000)
			#define AD9276_FULL_PWR_DOWN_VAL		(0b001)
			#define AD9276_STANDBY_VAL				(0b010)
			#define AD9276_RESET_VAL				(0b011)
			#define AD9276_CWMODE_VAL				(0b100)
			
	#define AD9276_CLK_REG                  (0x09) // default 0x01, turns the internal duty cycle stabilizer (DCS) on and off (global)
		#define AD9276_DCS_SHFT					(0)
		#define AD9276_DCS_MSK					(0x01<<AD9276_DCS_SHFT)
		
	#define AD9276_TESTIO_REG               (0x0D) // default 0x00, when this register is set, the test data is placed on the output pins in place of normal data (local, except for PN sequence)
		#define AD9276_USR_TESTMODE_SHFT		(6)
		#define AD9276_USR_TESTMODE_MSK			(0b11<<AD9276_USR_TESTMODE_SHFT)
			#define AD9276_USR_TESTMODE_OFF_VAL			(0b00)
			#define AD9276_USR_TESTMODE_SNGL_ALT_VAL	(0b01)
			#define AD9276_USR_TESTMODE_SNGL_ONCE_VAL	(0b10)
			#define AD9276_USR_TESTMODE_ALT_ONCE_VAL	(0b11)
		#define AD9276_RST_PN_LONG_GEN_SHFT				(5)
		#define AD9276_RST_PN_LONG_GEN_MSK				(0x01<<AD9276_RST_PN_LONG_GEN_SHFT)
		#define AD9276_RST_PN_SHRT_GEN_SHFT				(4)
		#define AD9276_RST_PN_SHRT_GEN_MSK				(0x01<<AD9276_RST_PN_SHRT_GEN_SHFT)
		#define AD9276_OUT_TEST_MODE_SHFT		(0)
		#define AD9276_OUT_TEST_MODE_MSK		(0b1111<<AD9276_OUT_TEST_MODE_SHFT)
			#define AD9276_OUT_TEST_OFF_VAL		(0b0000)
			#define AD9276_OUT_TEST_MIDSCL_SHRT_VAL		(0b0001)
			#define AD9276_OUT_TEST_FSP_SHRT_VAL		(0b0010)
			#define AD9276_OUT_TEST_FSN_SHRT_VAL		(0b0011)
			#define AD9276_OUT_TEST_CHCKBOARD_VAL		(0b0100)
			#define AD9276_OUT_TEST_PNSEQ_LONG_VAL		(0b0101)
			#define AD9276_OUT_TEST_PNSEQ_SHRT_VAL		(0b0110)
			#define AD9276_OUT_TEST_10_WORDTOG_VAL		(0b0111)
			#define AD9276_OUT_TEST_USR_INPUT_VAL		(0b1000)
			#define AD9276_OUT_TEST_10_BITTOG_VAL		(0b1001)
			#define AD9276_OUT_TEST_1xSYNC_VAL			(0b1010)
			#define AD9276_OUT_TEST_1BIT_HIGH_VAL		(0b1011)
			#define AD9276_OUT_TEST_MIX_BITFREQ_VAL		(0b1100)
			
	#define AD9276_GPO_OUT_REG              (0x0E) // default 0x00, values placed on GPO[0:3] pins (global)
		#define AD9276_GPO_SHFT					(0)
		#define AD9276_GPO_MSK					(0x0F<<AD9276_GPO_SHFT)
		
	#define AD9276_FLEX_CH_IN_REG           (0x0F) // default 0x30, antialiasing filter cutoff (global)
		#define AD9276_FLEX_CH_IN_SHFT			(4)
		#define AD9276_FLEX_CH_IN_MSK			(0x0F<<AD9276_FLEX_CH_IN_SHFT)
			#define AD9276_FLEX_CH_CUTOFF_43PCTG_FS	(0b0000)
			#define AD9276_FLEX_CH_CUTOFF_40PCTG_FS	(0b0001)
			#define AD9276_FLEX_CH_CUTOFF_37PCTG_FS	(0b0010)
			#define AD9276_FLEX_CH_CUTOFF_33PCTG_FS	(0b0011)
			#define AD9276_FLEX_CH_CUTOFF_30PCTG_FS	(0b0100)
			#define AD9276_FLEX_CH_CUTOFF_26PCTG_FS	(0b0101)
			#define AD9276_FLEX_CH_CUTOFF_23PCTG_FS	(0b0110)
			#define AD9276_FLEX_CH_CUTOFF_29PCTG_FS	(0b0111)
			#define AD9276_FLEX_CH_CUTOFF_27PCTG_FS	(0b1001)
			#define AD9276_FLEX_CH_CUTOFF_24PCTG_FS	(0b1010)
			#define AD9276_FLEX_CH_CUTOFF_22PCTG_FS	(0b1011)
			#define AD9276_FLEX_CH_CUTOFF_20PCTG_FS	(0b1100)
			#define AD9276_FLEX_CH_CUTOFF_18PCTG_FS	(0b1101)
			#define AD9276_FLEX_CH_CUTOFF_16PCTG_FS	(0b1110)
			
	#define AD9276_FLEX_OFST_REG            (0x10) // default 0x20, LNA force offset correction (local)
		#define AD9276_FLEX_OFST_MSK			(0b111111)
			#define AD9276_FLEX_OFST_LNA_BIAS_HI_VAL	(0b100000)
			#define AD9276_FLEX_OFST_LNA_BIAS_LO_VAL	(0b100001)
			
	#define AD9276_FLEX_GAIN_REG            (0x11) // default 0x06, LNA and PGA gain adjustment (global)
		#define AD9276_PGA_GAIN_SHFT			(2)
		#define AD9276_PGA_GAIN_MSK				(0b11<<AD9276_PGA_GAIN_SHFT)
			#define AD9276_PGA_GAIN_21dB_VAL		(0b00)
			#define AD9276_PGA_GAIN_24dB_VAL		(0b01)
			#define AD9276_PGA_GAIN_27dB_VAL		(0b10)
			#define AD9276_PGA_GAIN_30dB_VAL		(0b11)
		#define AD9276_LNA_GAIN_SHFT			(0)
		#define AD9276_LNA_GAIN_MSK				(0b11<<AD9276_LNA_GAIN_SHFT)
			#define AD9276_LNA_GAIN_15dB_VAL		(0b00)
			#define AD9276_LNA_GAIN_18dB_VAL		(0b01)
			#define AD9276_LNA_GAIN_21dB_VAL		(0b10)
			
	#define AD9276_BIAS_CURR_REG			(0x12) // default 0x08, LNA bias current adjustment (global)
		#define AD9276_BIAS_CURR_HIGH_VAL		(0b1000)
		#define AD9276_BIAS_CURR_MIDHIGH_VAL	(0b1001)
		#define AD9276_BIAS_CURR_MIDLOW_VAL		(0b1010)
		#define AD9276_BIAS_CURR_LOW_VAL		(0b1011)
		
	#define AD9276_OUT_MODE_REG				(0x14) // default 0x00, configures the outputs and the format of the data (bits[7:3] and bits[1:0] are global, bit 2 is local)
		#define AD9276_OUT_MODE_SHFT			(6)
		#define AD9276_OUT_MODE_MSK				(1<<AD9276_OUT_MODE_SHFT)
			#define AD9276_OUT_MODE_LVDS_ANSI_VAL	(0)
			#define AD9276_OUT_MODE_LVDS_LP_VAL		(1)
		#define AD9276_OUT_MODE_INVERT_EN_SHFT	(2)
		#define AD9276_OUT_MODE_INVERT_EN_MSK	(1<<AD9276_OUT_MODE_INVERT_EN_SHFT)
		#define AD9276_OUT_MODE_DAT_FORMAT_SHFT	(0)
		#define AD9276_OUT_MODE_DAT_FORMAT_MSK	(0b11<<AD9276_OUT_MODE_DAT_FORMAT_SHFT)
			#define AD9276_OUT_MODE_OFSTBIN_VAL		(0x00)
			#define AD9276_OUT_MODE_TWOSCOMP_VAL	(0x01)

	#define AD9276_OUT_ADJ_REG				(0x15) // default 0x00, determines lvds or other output properties. Primarily functions to set the LVDS span and common-mode levels in place of an external resistor (bits[7:1] are global; bit 0 is local)
		#define AD9276_OUT_ADJ_TERM_SHFT			(4)
		#define AD9276_OUT_ADJ_TERM_MSK				(0b11<<AD9276_OUT_ADJ_TERM_SHFT)
			#define AD9276_OUT_ADJ_TERM_000OHM_VAL		(0b00)
			#define AD9276_OUT_ADJ_TERM_200OHM_VAL		(0b01)
			#define AD9276_OUT_ADJ_TERM_100OHM_VAL		(0b10)
			#define AD9276_OUT_ADJ_TERM_100OHM_VAL_		(0b11) // the same as the other 100Ohms
		#define AD9276_OUT_ADJ_DCOFCO_DRV_STR_SHFT	(0)
		#define AD9276_OUT_ADJ_DCOFCO_DRV_STR_MSK	(1<<AD9276_OUT_ADJ_DCOFCO_DRV_STR_SHFT)
	
	#define AD9276_OUT_PHS_REG				(0x16) // default 0x03, on devices that utilize global clock divide, determines which phase of the divider output is used to supply the output clock. Internal latching is unaffected
		#define AD9276_OUT_PHS_000DEG_VAL		(0b0000)
		#define AD9276_OUT_PHS_060DEG_VAL		(0b0001)
		#define AD9276_OUT_PHS_120DEG_VAL		(0b0010)
		#define AD9276_OUT_PHS_180DEG_VAL		(0b0011)
		#define AD9276_OUT_PHS_240DEG_VAL		(0b0100)
		#define AD9276_OUT_PHS_300DEG_VAL		(0b0101)
		#define AD9276_OUT_PHS_360DEG_VAL		(0b0110)
		#define AD9276_OUT_PHS_420DEG_VAL		(0b0111)
		#define AD9276_OUT_PHS_480DEG_VAL		(0b1000)
		#define AD9276_OUT_PHS_540DEG_VAL		(0b1001)
		#define AD9276_OUT_PHS_600DEG_VAL		(0b1010)
		#define AD9276_OUT_PHS_660DEG_VAL		(0b1011)
		#define AD9276_OUT_PHS_660DEG1_VAL		(0b1100) // same as first 660 degrees
		#define AD9276_OUT_PHS_660DEG2_VAL		(0b1101) // same as first 660 degrees
		#define AD9276_OUT_PHS_660DEG3_VAL		(0b1110) // same as first 660 degrees
		#define AD9276_OUT_PHS_660DEG4_VAL		(0b1111) // same as first 660 degrees
	
	#define AD9276_FLEX_VREF_REG			(0x18) // default 0x00, select internal reference (recommended default) or external reference (global)
		#define AD9276_FLEX_VREF_INT_VAL		(0x00<<6)
		#define AD9276_FLEX_VREF_EXT_VAL		(0x01<<6)
	
	#define AD9276_USR_PATT1_LSB_REG		(0x19) // default 0x00, user defined pattern 1, LSB(global)
	#define AD9276_USR_PATT1_MSB_REG		(0x1A) // default 0x00, user defined pattern 1, MSB(global)
	#define AD9276_USR_PATT2_LSB_REG		(0x1B) // default 0x00, user defined pattern 2, LSB(global)
	#define AD9276_USR_PATT2_MSB_REG		(0x1C) // default 0x00, user defined pattern 2, MSB(global)
	
	#define AD9276_SRL_CNT_REG				(0x21) // default 0x00, serial stream control (global)
		#define AD9276_SRL_CNT_LSBFIRST_SHFT	(7)
		#define AD9276_SRL_CNT_LSBFIRST_MSK		(1<<AD9276_SRL_CNT_LSBFIRST_SHFT)
		#define AD9276_SRL_CNT_LO_ENC_SHFT		(3)
		#define AD9276_SRL_CNT_LO_ENC_MSK		(1<<AD9276_SRL_CNT_LO_ENC_SHFT)
		#define AD9276_SRL_CNT_BITLEN_SHFT		(0)
		#define AD9276_SRL_CNT_BITLEN_MSK		(0b111<<AD9276_SRL_CNT_BITLEN_SHFT)
			#define AD9276_SRL_CNT_BITLEN_12BITS_VAL	(0b000)
			#define AD9276_SRL_CNT_BITLEN_08BITS_VAL	(0b001)
			#define AD9276_SRL_CNT_BITLEN_10BITS_VAL	(0b010)
			#define AD9276_SRL_CNT_BITLEN_12BITS_VAL_	(0b011) // the same as previous 12-bits
			#define AD9276_SRL_CNT_BITLEN_14BITS_VAL	(0b100)
	
	#define AD9276_SRL_CH_STAT_REG			(0x22) // default 0x00, used to power down individual sections of a converter (local)
		#define AD9276_SRL_CH_OUT_RST_SHFT		(1)
		#define AD9276_SRL_CH_OUT_RST_MSK		(1<<AD9276_SRL_CH_OUT_RST_SHFT)
		#define AD9276_SRL_CH_OUT_PWRDOWN_SHFT	(0)
		#define AD9276_SRL_CH_OUT_PWRDOWN_MSK	(1<<AD9276_SRL_CH_OUT_PWRDOWN_SHFT)

	#define AD9276_FLEX_FILT_REG			(0x2B) // default 0x00, filter cutoff (global). (flp = low-pass filter cutoff frequency)
		#define AD9276_FLEX_FILT_AUTO_SHFT		(6)
		#define AD9276_FLEX_FILT_AUTO_MSK		(1<<AD9276_FLEX_FILT_AUTO_SHFT)
		#define AD9276_FLEX_FILT_HPF_MSK		(0x0F)
			#define AD9276_FLEX_FILT_HPF_04PCTG_FLP_VAL	(0b0000)
			#define AD9276_FLEX_FILT_HPF_09PCTG_FLP_VAL	(0b0001)
			#define AD9276_FLEX_FILT_HPF_13PCTG_FLP_VAL	(0b0010)
			#define AD9276_FLEX_FILT_HPF_17PCTG_FLP_VAL	(0b0011)
			#define AD9276_FLEX_FILT_HPF_20PCTG_FLP_VAL	(0b0100)
			#define AD9276_FLEX_FILT_HPF_24PCTG_FLP_VAL	(0b0101)
			#define AD9276_FLEX_FILT_HPF_29PCTG_FLP_VAL	(0b0110)
			#define AD9276_FLEX_FILT_HPF_32PCTG_FLP_VAL	(0b0111)
	
	// below this is not completed YET
	#define AD9276_ANA_IN_REG				(0x2C) //default 0x00, LNA active termination/input impedance (global). READ the datasheet
	#define AD9276_CW_DOPPL_IQ_REG			(0x2D)
