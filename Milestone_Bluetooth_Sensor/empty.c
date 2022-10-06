//////////////////////////////////////////////////////////////////////////////
// Last Updated: 10/05/2022 //////////////////////////////////////////////////
//                                                                          //
// Team Members: Sinan Kekic, Mohammed Kassif, Jiakang Du                   //
//                                                                          //
// Intent of Program:                                                       //
// The intent of this Program is to test the Bluetooth Feature. To do so,   //
// this program utilizes the UART driver of the TI Launchpad connected with //
// the HC-05 Module. The defining test is to send a signal to the HC-05     //
// Module with a Two Letter Identification (Defined Below). When the Module //
// receives the signal, it is passed to the Microcontroller, which turns    //
// produces an output signal. For this test, simple LED Output signals will //
// suffice.                                                                 //
//////////////////////////////////////////////////////////////////////////////

          ////////////////////////////////////////
          // Command Definition:                //
          // Two Letter ID - Function           //
          //                                    //
          // P0 - PWM Disable                   //
          // RF - Right Wheel Forward           //
          // RR - Right Wheel Reverse           //
          // LF - Left Wheel Forward            //
          // LR - Left Wheel Reverse            //
          // LG - Light Read (get)              //
          // GO - PID Go                        //
          // R0 - Read Front Distance Sensor    //
          // R1 - Read Right Distance Sensor    //
          // TD - Toggle Data Acquisition       //
          // DS - Post Drive Semaphore          //
          // ES - Emergency Stop                //
          // DC - Drive Clock Start             //
          ////////////////////////////////////////

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


void ConfigClock(void)
{
    // Set Clock Signal
    SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ |
                               SYSCTL_OSC_MAIN);
}

void ConfigUART(void)
{
    //
    // Enable the GPIO Peripheral used by the UART.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);

    //
    // Enable UART5
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART5);

    // Config GPIO Pins for UART Mode
    GPIOPinTypeUART(GPIO_PORTE_BASE, GPIO_PIN_4 | GPIO_PIN_5);
    //
    // Configure GPIO Pins as RX and TX
    GPIOPinConfigure(GPIO_PE4_U5RX);
    GPIOPinConfigure(GPIO_PE5_U5TX);

    // Set Clock for UART
    UARTConfigSetExpClk(UART5_BASE, SysCtlClockGet(), 9600, (UART_CONFIG_WLEN_8|UART_CONFIG_STOP_ONE|UART_CONFIG_PAR_NONE));
}

void ConfigLED(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF));
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);
        // LED
}

// uint8_t is unsigned char ~ is 8 bits,
// uint16_t is unsigned short int,
// uint32_t is unsigned int
// uint64_t is unsigned long long
// int32_t is signed int

// commandFunction structure relevant for GPIOPinWrite Function. Carries all values.
// may need to put a flag function/member value for commands with multiple functions (such as Emergency turning off all sensors)
typedef struct {
    uint32_t commandPortBase; // Port Base for GPIOPinWrite ~~~ Port Base Example: (E) 0x40024000,
    uint16_t commandPin; // Pin Location for GPIOPinWrite
    uint16_t commandWrite; // Write Value for GPIOPinWrite
    uint16_t noCommandFlag; // Flag, High if no existing command ~ Does not Write if no Command Function entered
} commandFunction;


// Key + Data structure for interpreting 2-letter commands.
// Command Key is a char pointer which will be used to locate struct commandFunction member
typedef struct {
    unsigned char *commandKey;
    commandFunction commandFunc;
} commandList;


// Lookup table
commandList* lookupTable(commandList* commands, unsigned int size, const unsigned char *commandKey)
{
    unsigned int i;
    for(i = 0; i<size; i++)
    {
        if(strcmp(commands[i].commandKey, commandKey) == 0) // Warning here is strcmp takes char* instead of unsigned char*)
        {
            return &commands[i];
        }
    }
    commandList* noCommand = {"", {0, 0, 0, 1}};
    return noCommand;
    // Warning: add return if no function is matched to Two Letter input
}

/*
 *  ======== main ========
 */
int main(void)
{
        ConfigClock();
        ConfigUART();
        ConfigLED();

        unsigned char inUART[2];
        unsigned char noCommand[20] = "No Command Executed";

        commandList commands[] =
        {
          {"P0", {GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_PIN_1, 0}}, // Turns on Red LED for Simulation
          {"RF", {GPIO_PORTF_BASE, GPIO_PIN_1, 0, 0}},          // Turns off Red LED
          {"RR", {GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_PIN_2, 0}}, // Turns on Blue LED
          {"LF", {GPIO_PORTF_BASE, GPIO_PIN_2, 0, 0}},          // Turns off Blue LED
          {"LR", {GPIO_PORTF_BASE, GPIO_PIN_3, GPIO_PIN_3, 0}}, // Turns on Green LED
          {"LG", {GPIO_PORTF_BASE, GPIO_PIN_3, 0, 0}},          // Turns off Green LED
          {"GO", {0, 0, 0, 1}}, // keep at 1 for flag [High flag if Command Does not Exist]
          {"R0", {0, 0, 0, 1}}, // ...
          {"R1", {0, 0, 0, 1}}, // ...
          {"TD", {0, 0, 0, 1}}, // ...
          {"DS", {0, 0, 0, 1}}, // ...
          {"ES", {0, 0, 0, 1}}, // ...
          {"DC", {0, 0, 0, 1}}, // keep at 1 for flag [High flag if Command Does not Exist]
          //
          // { "[Two Letter Identifier]", {GPIO_PORT@_BASE, GPIO_PIN_#, [Write Value], [Low flag if Command Exists] }}

        };

        // Finds the number of members in commands array by sizeof
        unsigned int num_items = sizeof(commands) / sizeof(commandList);

        while(1)
        {
            // receives user input from UART Terminal from UARTCharGet
            // Echo's user input from UARTCharPut
            inUART[0] = UARTCharGet(UART5_BASE);
            UARTCharPut(UART5_BASE, inUART[0]);
            inUART[1] = UARTCharGet(UART5_BASE);
            UARTCharPut(UART5_BASE, inUART[1]);

            // Creates Space for a new line
            UARTCharPut(UART5_BASE, '\n');
            UARTCharPut(UART5_BASE, '\r');


            // map Function (Lookup Table)
            commandList* function = lookupTable(commands, num_items, inUART);

            // executes command (needs if statement to print no command if Flag for no return found
            if(function->commandFunc.noCommandFlag)
            {
                int count;
                for(count = 0; count < 19; count++)
                {
                    UARTCharPut(UART5_BASE, noCommand[count]);
                }
                UARTCharPut(UART5_BASE, '\n');
                UARTCharPut(UART5_BASE, '\r');
            }
            else
            {
                GPIOPinWrite(function->commandFunc.commandPortBase, function->commandFunc.commandPin, function->commandFunc.commandWrite);
            }

        }

    return (0);
}
