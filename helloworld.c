//References:  [1] The Zynq book tutorial, link: http://www.zynqbook.com/download-tuts.html
//	       [2] Mauricio, link: https://github.com/mrodriguezalas/system-on-chip-labs/tree/main/zynq_lab1/src
//	       [3] Teacher Levi Marien, KU Leuven, System on Chip course,



//----------------------------------------------------
// LOOK AT THIS CODE FROM THE ZYNQ PS PERSPECTIVE
// As if we were the Zynq chip, who receives this code as
// a task to carry out.
//----------------------------------------------------

/* Include Files */
#include "platform.h"
#include "xparameters.h"
#include "xgpio.h"
#include "xstatus.h"
#include "xil_printf.h"
#include "sleep.h"



//Interfaces of GPIO 0:
#define LED_CHANNEL 1	// GPIO port/interface for LEDs

//Instance of the GPIO Device Driver.

XGpio Gpio0;

void WriteLeds(int ledData)
{
	XGpio_DiscreteWrite(&Gpio0, LED_CHANNEL, ledData);
	usleep(500*1000);
	return;
}

/* Main function. */
int main(void){

	u32 led= 0x0000000F;
	//oxF = ob1111
	//oxA = ob1010


	// Variable to store the address of Gpio(s) predefined configuration
	XGpio_Config *cfg_ptr0;


	// Initialize xilinx platform
	xil_printf("initialization of the platform\n\r");
	init_platform();

	//----------------------------------------------------
	// INITIALIZE and CONFIGURE (or SETUP) the GPIO(s):
	//----------------------------------------------------
	//1. INTIALIZATION STEP:
		//1.1 Look up for the predefined configuration of the GPIO 0:
	cfg_ptr0 = XGpio_LookupConfig(XPAR_AXI_GPIO_0_DEVICE_ID); // GPIO 0

		//1.2 Initialize GPIO 0:
	XGpio_CfgInitialize(&Gpio0, cfg_ptr0, cfg_ptr0->BaseAddress); // GPIO 0

	//2. CONFIGURE STEP
	//   Configure the interfaces of GPIO 0 in software
	//	 to be compatible with the hardware design of those Gpio(s):
	//On GPIO 0:
	XGpio_SetDataDirection(&Gpio0, LED_CHANNEL, 0x00000FF5); // channel 1 (leds), output mode
	// 0x5 = 0b0101


	//----------------------------------------------------
	// USING THE GPIO(s).
	//----------------------------------------------------
	while(1)
	{
		WriteLeds(led);

	}

	// Cleanup the xilinx platform
	xil_printf("cleanup\n\r");
	cleanup_platform();

	return 0;
}
