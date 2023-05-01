/*
 * Author: Tran Le Phuong Lan
 * References: [1] The Zynq book tutorial
 *			   [2] Teacher Levi Marien, KU Leuven, System on Chip course,
 *			   [3] SDK Xilinx examples, xgpio_example_1.c
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

//---
//For interrupt mode
#include "xscugic.h" // GIC (Generic Interrupt Controller, residing in the Processing System).
					 // GIC is in charging of handling the interrupt signals sent to the Processing System.
					 // This is GIC driver provided by SDK Xilinx
#include "xil_exception.h"
//----


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

//-----
// GIC declaration & definition:
//-----
#define INTR_ID_GPIO_0 XPAR_FABRIC_AXI_GPIO_0_IP2INTC_IRPT_INTR //IntrId

//The Instance of the Interrupt Controller Driver
XScuGic INTCInst;

//Gpio 0 flags:
int Gpio0Error=0;

//Function prototypes:
void gic_intr_setup(XScuGic* GicInstancePtr, int GicInstanceID);
void gpio_0_intr_handler(void *InstancePtr);

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
	// SET UP THE INTERRUPT SERVICE:
	//----------------------------------------------------
	gic_intr_setup(&INTCInst, XPAR_PS7_SCUGIC_0_DEVICE_ID);

	// Set the type of interrupt trigger (rising-edge or high-level)
	// void XScuGic_SetPriorityTriggerType(XScuGic *InstancePtr, u32 Int_Id, u8 Priority, u8 Trigger)
	// @param	Trigger:  b01   Active HIGH level sensitive
	//					  b11   Rising edge sensitive
	XScuGic_SetPriorityTriggerType(&INTCInst, INTR_ID_GPIO_0, 0x08, 0b11);

	// Connect GPIO interrupt to Interrupt controller, lab 2
	XScuGic_Connect(&INTCInst,
				 	INTR_ID_GPIO_0, //GIC - GPIO interrupt connection ID
					(Xil_ExceptionHandler)gpio_0_intr_handler,
		  			(void *)&Gpio0);

	// Enable GIC setup interrupt channel:
	XScuGic_Enable(&INTCInst, INTR_ID_GPIO_0);

	// Enable GPIO interrupt to be active:
	// XGPIO_IR_CH1_MASK: IntrMask
	XGpio_InterruptEnable(&Gpio0, XGPIO_IR_CH2_MASK); // XGPIO_IR_CH2_MASK : interrupt mask for channel 2 of Gpio
													  // Why channel 2? because the interrupt signal can only be generated by input port!
													  // In this case, the input port is the Switch (2bits) port/interface/channnel 
	XGpio_InterruptGlobalEnable(&Gpio0);
	//---------------------



	//----------------------------------------------------
	// MAIN APPLICATION:
	//----------------------------------------------------
	//GPIO
	u8 SwVal;

	// Loop for operation.
	while (1)
		{
			//---
			// Poll the leds
			//---
			WriteLeds(0b000001000); // turn on 4th led
			WriteLeds(0b000000000); // turn off 4th led

			//check the interrupt
			if(!Gpio0Error)
			{
				SwVal = XGpio_DiscreteRead(&Gpio0, SW_CHANNEL);
				xil_printf("Switch value inside the loop: %d.\n", SwVal);
			}
			else
			{
				xil_printf("Gpio interrupt error (printed inside the loop)\n");
			}
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



//-----
// GIC-related functions:
//-----
void gic_intr_setup(XScuGic* GicInstancePtr, int GicInstanceID)
{

		XScuGic_Config *IntcConfig;
		//----------------------------------------------------
		// INITIALIZE and CONFIGURE (or SETUP) the GIC in the PS part of the Zynq chip
		//----------------------------------------------------
		// 1. Initialisation: Interrupt controller, GIC,
		IntcConfig = XScuGic_LookupConfig(GicInstanceID);
		XScuGic_CfgInitialize(GicInstancePtr, IntcConfig, IntcConfig->CpuBaseAddress);


		// 2. Configuration/Setup: Interrupt controller, GIC,
		Xil_ExceptionInit();
		Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
		  			 	 	 	 	(Xil_ExceptionHandler)XScuGic_InterruptHandler,
									GicInstancePtr);
		Xil_ExceptionEnable();

}

// AXI GPIO 0, interrupt routine
void gpio_0_intr_handler(void *InstancePtr)
{

	if(InstancePtr == &Gpio0)
	{
		xil_printf("Interrupt routine generated by AXI GPIO 0.\n");
		//Gpio0Error =1;
	}
	else
	{
		Gpio0Error =1;
	}



	//-----
	//Service to the interrupt signal
	u8 SwVal;
	SwVal = XGpio_DiscreteRead(&Gpio0, SW_CHANNEL);

	int i=0;
	for (i=0; i<5; i++)
	{
		WriteLeds(SwVal); // turn on leds corresponding to the read-in switch value.
		WriteLeds(0b000000000); // turn off all leds
	}
	//-----

    // Clearing this interrupt signal from pending list.
	// Because it has now been served.
    (void)XGpio_InterruptClear(&Gpio0, XGPIO_IR_CH2_MASK);


    xil_printf("Out of the interrupt routine of AXI GPIO 0.\n");
}
