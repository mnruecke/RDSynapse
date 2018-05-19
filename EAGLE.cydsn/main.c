/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#include "project.h"
#include <stdio.h>
#include <string.h>

#define FALSE    0
#define NUMBER   1
#define COMMAND  2


#define KEY_SINGLE_SHOT     's'
#define KEY_CONTINUOUS      'c'
#define KEY_HALT_CONTINUOUS 'h'

void init_psoc(void);
void run_uart_interface(void);

char sms[80];

// main---------------------------------------------------------
int main(void)
{
    init_psoc();

    for(;;)
    {   
        run_uart_interface();
        
    }
}
//END main-------------------------------------------------------


void init_psoc(void)
{
    CyGlobalIntEnable; /* Enable global interrupts. */

    UART_Start();
    PWM_SequDuty_Start();
    
    UART_PutString("PSoC now running!\n\r"); 
    UART_PutString("\n\rMenue options:\n\r"); 
    UART_PutString("-------------------------------------------\n\r"); 
    sprintf(sms, "[%c]ingle measurement\n\r", KEY_SINGLE_SHOT);
        UART_PutString(sms); 
    sprintf(sms, "[%c]ontinuous mode\n\r", KEY_CONTINUOUS );
        UART_PutString(sms);
    sprintf(sms, "[%c]alt continuous mode\n\r", KEY_HALT_CONTINUOUS );
        UART_PutString(sms);
    UART_PutString("[XXX] (three digit number) f_Y = f_Y0 + XXX"); 
}

void run_uart_interface(void)
{
    char puttyIn;
    uint8 nextValue=0;
    uint8 nextValueValid = FALSE;
    double frequ;
    
    if( UART_GetRxBufferSize() > 0)
    {
        // START - command line control
        nextValue=0;
        uint8 nextValueValid = FALSE;
        puttyIn = UART_GetChar();
        if( puttyIn == KEY_SINGLE_SHOT )
        {
            // Single shot command
            SW_Write(0u);
            CyDelay(1u);
            SW_Write(1u);
            nextValueValid = COMMAND;
        }
        else if( puttyIn == KEY_CONTINUOUS )
        {
            // Continuous pulsing
            SW_Write(0u);
            nextValueValid = COMMAND;
        }
        else if( puttyIn == KEY_HALT_CONTINUOUS )
        {
            // Stop continuous pulsing
            SW_Write(1u);
            nextValueValid = COMMAND;
        }
        else if( ((puttyIn - '0') >= 0) && ((puttyIn-'9') <=9) )
        {
            // 3. Digit
            nextValue = 100 * (puttyIn - '0');
            UART_PutChar(puttyIn);
            // 2. Digit
            while( UART_GetRxBufferSize() == 0);
            puttyIn = UART_GetChar();
            if( ((puttyIn - '0') >= 0) && ((puttyIn-'9') <=9) )
            {
                nextValue = nextValue + 10 * (puttyIn - '0');
                UART_PutChar(puttyIn);
                // 1. Digit
                while( UART_GetRxBufferSize() == 0);
                puttyIn = UART_GetChar();
                if( ((puttyIn - '0') >= 0) && ((puttyIn-'9') <=9) )
                {
                    nextValue = nextValue + (puttyIn - '0');
                    UART_PutChar(puttyIn);    
                    nextValueValid = NUMBER;
                }
                else nextValueValid = FALSE;
            }
            else nextValueValid = FALSE;    
        }
        else nextValueValid = FALSE;
        // END - command line entry control  
        
        // set new value
        if( nextValueValid == NUMBER )
        {
            accu_word_Y_Write( nextValue );
            frequ = ((double)72000000/(double)65536*((double)4400+(double)nextValue));
            sprintf(sms,"\n\rNew Y value is set to: %5d\n\r(X=5062500 Hz @ P1.2, Y=%6d @ P1.4) \n\r", nextValue, (int)frequ);
            UART_PutString(sms);
            nextValueValid = FALSE;
        }
        else if( nextValueValid == COMMAND )
        {
            switch(puttyIn){
                case KEY_SINGLE_SHOT    : sprintf(sms, "\n\rSingle shot\n\r"); break;
                case KEY_CONTINUOUS     : sprintf(sms, "\n\rContinuous mode\n\r"); break;
                case KEY_HALT_CONTINUOUS: sprintf(sms, "\n\rHalt continuous mode\n\r"); break;
            }    
            UART_PutString(sms);
            nextValueValid = FALSE;  
        }
        else
        {
            UART_PutString("\n\rNot a valid entry!\n\r");
        }
        
        UART_ClearRxBuffer();
    }
}




/* [] END OF FILE */
