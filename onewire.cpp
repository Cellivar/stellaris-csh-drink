/**
 * @file onewire.cpp
 *
 * CSH Drink Controller
 */

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "utils/uartstdio.h"
#include "utils/ustdlib.h"
#include "utils/ustdlib.c"

#include <vector>
#include <algorithm>

#include "stellaris-onewire/OneWireMaster.h"

//*****************************************************************************
//
// The error routine that is called if the driver library encounters an error.
//
//*****************************************************************************
#ifdef DEBUG
void
__error__(char *pcFilename, unsigned long ulLine)
{
}
#endif


void InitCPU(void)
{
	// Setup for 16MHZ external crystal, use 200MHz PLL and divide by 4 = 50MHz
	ROM_SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ |
			  SYSCTL_OSC_MAIN);
}

void InitUART(void)
{
	// Initialize the UART.
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
	ROM_GPIOPinConfigure(GPIO_PA0_U0RX);
	ROM_GPIOPinConfigure(GPIO_PA1_U0TX);
	ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
	UARTStdioInit(0);
}


int main(void)
{
	InitCPU();
	InitUART();

	// Pin objects used for debugging/status/etc.
	DigitalIOPin LEDRED(SYSCTL_PERIPH_GPIOF, GPIO_PORTF_BASE, GPIO_PIN_1);
	DigitalIOPin LEDBLUE(SYSCTL_PERIPH_GPIOF, GPIO_PORTF_BASE, GPIO_PIN_2);
	DigitalIOPin LEDGREEN(SYSCTL_PERIPH_GPIOF, GPIO_PORTF_BASE, GPIO_PIN_3);

	// OneWire controller object
	OneWireMaster OWM(OW_SPEED_STANDARD, SYSCTL_PERIPH_GPIOA, GPIO_PORTA_BASE, GPIO_PIN_5);

	// Mark that we've started a new session
	UARTprintf("\n\nSTART\n\n");

	OWM.Search();

	char buffchar[50];
	UARTprintf("\nSEARCH DONE\n");
	UARTprintf("Device table:\n");

	for (int d = 0; d < OWM.devices.size(); ++d)
	{
		UARTprintf("Device: ");
		for (int i = 0; i < OWM.devices[d].size(); ++i)
		{
			usprintf(buffchar, "%X", OWM.devices[d][i]);
			UARTprintf(buffchar);
			UARTprintf(" ");
		}
		UARTprintf("\n");
	}

	return 0;

}
