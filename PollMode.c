/*
 *
 *References: [1] The Zynq book tutorial
 *			  [2] Teacher Levi Marien, KU Leuven, System on Chip course,
 */


//----------------------------------------------------
// LOOK AT THIS CODE FROM THE ZYNQ CHIP PERSPECTIVE
// As if we were the Zynq chip, who receives this code as
// a task to carry out.
//----------------------------------------------------

/* Include Files */
# include <stdio.h>
#include "platform.h"
#include "xstatus.h"
#include "xparameters.h" //must have in every SDK project
#include "xgpio.h" // axi gpio driver, provided by SDK Xilinx
#include "xil_printf.h"
#include "sleep.h"


//-----
// AXI GPIO declaration & definition:
//-----
//Interfaces of GPIO 0:
#define LED_CHANNEL 1	// GPIO port/interface for LEDs
#define SW_CHANNEL 2	// GPIO port/interface for LEDs

//Instance of the GPIO Device Driver.
XGpio Gpio0;

//Function prototypes:
void WriteLeds(int ledData);
void Gpio_instance_initialization(XGpio* GpioInstance, int GpioInstanceID);


/*
 *	Main function.
 */
int main(void){

	// Initialize xilinx platform
	xil_printf("initialization of the platform\n\r");
	init_platform();


	//----------------------------------------------------
	// INITIALIZE and CONFIGURE (or SETUP) the GPIO(s):
	//----------------------------------------------------
	//1. INTIALIZATION STEP:
	Gpio_instance_initialization(&Gpio0, XPAR_AXI_GPIO_0_DEVICE_ID);


	//2. CONFIGURE STEP
	//   Configure the interfaces of GPIO 0 and GPIO 1 in software
	//	 to be compatible with the hardware design of those Gpio(s):
	//On GPIO 0:
	XGpio_SetDataDirection(&Gpio0, LED_CHANNEL, 0x0); // channel 1 (leds), output mode
	XGpio_SetDataDirection(&Gpio0, SW_CHANNEL, 0xF); // channel 2 (switch), input mode



	//----------------------------------------------------
	// MAIN APPLICATION:
	//----------------------------------------------------
	//GPIO
	u8 SwVal;

	// Loop for operation.
	while (1)
		{
			
			//---
			// Poll the switches
			//---
			SwVal = XGpio_DiscreteRead(&Gpio0, SW_CHANNEL);

			int i=0;
			for (i=0; i<5; i++)
			{
				WriteLeds(SwVal); // turn on leds corresponding to the read-in switch value.
				WriteLeds(0b000000000); // turn off all leds
			}
			
			//---
			// Poll the leds
			//---
			WriteLeds(0b000001000); // turn on 4th led
			WriteLeds(0b000000000); // turn off 4th led
		}



	// Cleanup the xilinx platform
	xil_printf("cleanup\n\r");
	cleanup_platform();

	return 0;
}

//-----
// AXI GPIO-related functions:
//-----
void Gpio_instance_initialization(XGpio* GpioInstance, int GpioInstanceID)
{
	XGpio_Config *cfg_ptr;
	//1.1 Look up for the predefined configuration of the GPIO:
	cfg_ptr = XGpio_LookupConfig(GpioInstanceID);

	//1.2 Initialize the GPIO:
	XGpio_CfgInitialize(GpioInstance, cfg_ptr, cfg_ptr->BaseAddress);

	return;
}

void WriteLeds(int ledData)
{
	XGpio_DiscreteWrite(&Gpio0, LED_CHANNEL, ledData);
	usleep(500*1000);
	return;
}