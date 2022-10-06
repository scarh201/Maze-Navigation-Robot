//////////////////////////////////////////////////////////////////////////////
// Last Updated: 10/05/2022 //////////////////////////////////////////////////
//                                                                          //
// Team Members: Sinan Kekic, Mohammed Kassif, Jiakang Du                   //
//                                                                          //
// Intent of Program:                                                       //
// The intent of this Program is to test the IR Distance Sensors. To do so, //
// This program utilizes the UART Driver of the Device and a Serial Port    //
// Connection to output the value of distance between the Sensor and a      //
// nearby object.                                                           //
//////////////////////////////////////////////////////////////////////////////

/* XDCtools Header files */
#include <xdc/std.h>
#include <xdc/runtime/System.h>


/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>


/* TI-RTOS Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/UART.h>

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"
#include "string.h"
#include "driverlib/systick.h"

#include "grlib/grlib.h"
#include "drivers/cfal96x64x16.h"

#include <math.h>

#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
}
#endif

************************************************************************

int finddistance(){
    uint32_t pui32ADC0Value[1];
    ADCProcessorTrigger(ADC0_BASE, 3);
                  while(!ADCIntStatus(ADC0_BASE, 3, false))  {}
                  ADCIntClear(ADC0_BASE, 3);
                  ADCSequenceDataGet(ADC0_BASE, 3, pui32ADC0Value);
                  int distance= 14871*pow(pui32ADC0Value[0], -.995); //this eq converts ADC value to cm
                  return distance;
}




void ConfigureADC(){

    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
        SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
        GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_3);
        ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_PROCESSOR, 0);
      ADCSequenceStepConfigure(ADC0_BASE, 3, 0, ADC_CTL_CH0 | ADC_CTL_IE |
                                 ADC_CTL_END);

        ADCSequenceEnable(ADC0_BASE, 3);
        ADCIntClear(ADC0_BASE, 3);
}



void
ConfigureUART(void)
{
    //
    // Enable the GPIO Peripheral used by the UART.
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    //
    // Enable UART0
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    //
    // Configure GPIO Pins for UART mode.
    //
    ROM_GPIOPinConfigure(GPIO_PA0_U0RX);
    ROM_GPIOPinConfigure(GPIO_PA1_U0TX);
    ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    //
    // Use the internal 16MHz oscillator as the UART clock source.
    //
    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);

    //
    // Initialize the UART for console I/O.
    //
    UARTStdioConfig(0, 115200, 16000000);
}



int
main(void)
{
   // Set Crystal Clock
    ROM_SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ |
                       SYSCTL_OSC_MAIN);

    // Configure UART and ADC
    ConfigureUART();
    ConfigureADC();

    while(1){
        // Continously Update Distance on UART Port
        UARTprintf("distance[CM] = %4d\r", finddistance());
               SysCtlDelay(SysCtlClockGet() / 12);
    }
}
