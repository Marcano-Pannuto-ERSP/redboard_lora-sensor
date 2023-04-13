// SPDX-License-Identifier: Apache-2.0
// Copyright: Gabriel Marcano, 2023

#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"

#include <string.h>
#include <assert.h>

#include <uart.h>
#include <adc.h>
#include <spi.h>
#include <lora.h>

#define CHECK_ERRORS(x)\
	if ((x) != AM_HAL_STATUS_SUCCESS)\
	{\
		error_handler(x);\
	}

static void error_handler(uint32_t error)
{
	(void)error;
	for(;;)
	{
		am_devices_led_on(am_bsp_psLEDs, 0);
		am_util_delay_ms(500);
		am_devices_led_off(am_bsp_psLEDs, 0);
		am_util_delay_ms(500);
	}
}

static struct uart uart;
static struct adc adc;

int main(void)
{
	// Prepare MCU by init-ing clock, cache, and power level operation
	am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_SYSCLK_MAX, 0);
	am_hal_cachectrl_config(&am_hal_cachectrl_defaults);
	am_hal_cachectrl_enable();
	am_bsp_low_power_init();
	am_hal_sysctrl_fpu_enable();
	am_hal_sysctrl_fpu_stacking_enable(true);

	// Init UART
	uart_init(&uart, UART_INST0);

	// Initialize the ADC.
	adc_init(&adc);

	// After init is done, enable interrupts
	am_hal_interrupt_master_enable();

	// Print the banner.
	am_util_stdio_terminal_clear();
	am_util_stdio_printf("Hello World!\r\n\r\n");

	// Trigger the ADC to start collecting data
	adc_trigger(&adc);

	uint32_t rx_data = 0;
	uint32_t tx_data = 0x80;
	struct lora lora;
	lora_init(&lora, 915000000);
	lora_standby(&lora);
	lora_set_spreading_factor(&lora, 7);
	lora_set_coding_rate(&lora, 1);
	lora_set_bandwidth(&lora, 0x7);
	//lora_receive_mode(&lora);


	// Wait here for the ISR to grab a buffer of samples.
	while (1)
	{
		// Print the battery voltage and temperature for each interrupt
		//
		uint32_t data = 0;
		if (adc_get_sample(&adc, &data))
		{
			// The math here is straight forward: we've asked the ADC to give
			// us data in 14 bits (max value of 2^14 -1). We also specified the
			// reference voltage to be 1.5V. A reading of 1.5V would be
			// translated to the maximum value of 2^14-1. So we divide the
			// value from the ADC by this maximum, and multiply it by the
			// reference, which then gives us the actual voltage measured.
			const double reference = 1.5;
			double voltage = data * reference / ((1 << 14) - 1);

			/*for (int i = 0; i < 0x30; ++i)
			{
				am_util_stdio_printf("Reg %02X: Value: %02X\r\n", i, lora_get_register(&lora, i));
			}*/
			
			int voltage_int = voltage*10000;
			unsigned char buffer[64]; // = "Hello World!!!\r\n";	//changed from 32
			//sprintf(buffer, "%f = Internal voltage", voltage);
			sprintf(buffer, "Internal voltage = %d", voltage_int);
			//debug
			am_util_stdio_printf(buffer);

			am_util_stdio_printf("Reg %02X: Value: %02X\r\n", 1, lora_get_register(&lora, 1));
			lora_send_packet(&lora, buffer, strlen(buffer));
			if (lora_rx_amount(&lora))
			{
				am_util_stdio_printf("length %i\r\n", lora_rx_amount(&lora));
				lora_receive_packet(&lora, buffer, 32);
				am_util_stdio_printf("Data: %s\r\n", buffer);
			}
			/*
			am_util_stdio_printf(
				"voltage = <%.3f> (0x%04X) ", voltage, data);

			am_util_stdio_printf("\r\n");
			*/
		}

		// Sleep here until the next ADC interrupt comes along.
		am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);
	}
}
