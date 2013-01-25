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



int main(void)
{
	// Setup for 16MHZ external crystal, use 200MHz PLL and divide by 4 = 50MHz
	ROM_SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ |
			  SYSCTL_OSC_MAIN);

	// Pin objects used for debugging/status/etc.
	DigitalIOPin LEDRED(SYSCTL_PERIPH_GPIOF, GPIO_PORTF_BASE, GPIO_PIN_1);
	DigitalIOPin LEDBLUE(SYSCTL_PERIPH_GPIOF, GPIO_PORTF_BASE, GPIO_PIN_2);
	DigitalIOPin LEDGREEN(SYSCTL_PERIPH_GPIOF, GPIO_PORTF_BASE, GPIO_PIN_3);

	// OneWire controller object
	OneWireMaster OWM(OW_SPEED_STANDARD, SYSCTL_PERIPH_GPIOA, GPIO_PORTA_BASE, GPIO_PIN_5);

	// Initialize the UART.
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
	ROM_GPIOPinConfigure(GPIO_PA0_U0RX);
	ROM_GPIOPinConfigure(GPIO_PA1_U0TX);
	ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
	UARTStdioInit(0);

	// Mark that we've started a new session
	UARTprintf("\n\nSTART\n\n");

	OWM.WaitUS(10000);

	char buffchar[50];
	int buffNum = 0;
	std::vector<unsigned char> device;

	// Clear the bus, display the reset status
	UARTprintf("Reset returned: ");
	usprintf(buffchar, "%i", OWM.Reset());
	UARTprintf(buffchar);
	UARTprintf("\n");

	OWM.WriteByte(OW_SEARCH_ROM);	// Run the search ROM command

	unsigned char bit, bitComp;

	for (int i = 0; i < 64; ++i)
	{
		OWM.WaitUS(100);

		bit = OWM.ReadBit();
		bitComp = OWM.ReadBit();

		if ((bit == 1) && (bitComp == 1))
		{
			// Both values are 1, indicating that something went wrong, we should have
			// gotten at least some signal. We're either down the wrong path, or there's nothing
			// on the line. Either way, hang up.
			break; // Something bad happened
		}
		else if (bit != bitComp)
		{
			// Two different values, meaning this single value is the one to take

			OWM.WriteBit(bit);
			buffNum >>= 1;
			bit <<= 7;
			buffNum |= bit;

		}
		else if ((bit == 0) && (bitComp == 0))
		{
			// Two different numbers. For debugging purposes, choose the 0 path and carry on
			OWM.WriteBit(0);
			buffNum >>= 1;
			bit <<= 7;
			buffNum |= 0x00;
			UARTprintf("Found junction, chose 0\n");
		}


		if ((i % 8) == 7)	// End of a byte, add it to the vector for the device
		{
			UARTprintf("Byte: ");
			usprintf(buffchar, "%X", buffNum);
			UARTprintf(buffchar);
			UARTprintf("\n");

			device.push_back(buffNum);
			buffNum = 0;
		}


	}


	UARTprintf("DONE\n");
	UARTprintf("Device table:\n");


	for (int i = 0; i < device.size(); ++i)
	{
		usprintf(buffchar, "%X", device[i]);
		UARTprintf(buffchar);
		UARTprintf(" ");
	}

	int testval = 0;

	while(1)
	{
		testval = OWM.Reset();
		LEDRED.Write(testval);
		OWM.WaitUS(980000);
		LEDRED.Write(0);
		OWM.WaitUS(20000);

	}

	return 0;

}
