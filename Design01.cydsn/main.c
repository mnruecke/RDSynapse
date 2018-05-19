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
#include <project.h>
#include <stdio.h>
#include <stdlib.h>

#define POTI_ADC_0      0
#define POTI_ADC_1      1
#define NUM_OF_AVG	    128
#define SAMPLING_TIME	100
#define UPDATE_TIMER	500

// Dictionary: ADC-value <-> Program Modi
#define RUN_FROM_EEPROM 0
#define SET_PERIOD_CH1  300
#define SET_PERIOD_CH2  1500
#define SET_DUTY_CYCLE1 2500
#define SET_DUTY_CYCLE2 3500
#define WRITE_TO_EEPROM 3900

// Characteristic times (unit = 1/(79 MHz))
#define PERIOD_0        150	
#define RUN_TIME_PROG   10000
#define RUN_TIME_NORMAL 50000
#define SEQUENCE_LENGTH 20

    
char msg[80];

uint adc_0=0;
uint adc_1=0;
uint shift_reg_value = 0;
uint avg_count=0;


void initialize_hardware(void);


CY_ISR( isr_1_handle )
{
    CyDelay(10);
    PWM_SEQU_WritePeriod( RUN_TIME_PROG );
    
    while( SW_Read() == 0 )
    {
            CyDelayUs( SAMPLING_TIME );
            avg_count++;
            
            adc_0 = ((NUM_OF_AVG-1) * adc_0 + ADC_Poti_GetResult16( POTI_ADC_0 )) / NUM_OF_AVG; // setting programming mode
            adc_1 = ((NUM_OF_AVG-1) * adc_1 + ADC_Poti_GetResult16( POTI_ADC_1 )) / NUM_OF_AVG; // setting parameter values
            
            if(avg_count > UPDATE_TIMER)
            {
                avg_count = 0;
                
                // generate blink sequence as mode indicator:
                if(adc_0 > RUN_FROM_EEPROM)     shift_reg_value  = 0b0;       // 1
                if(adc_0 > SET_PERIOD_CH1)      shift_reg_value  = 0b01;      // 2
                if(adc_0 > SET_PERIOD_CH2)      shift_reg_value  = 0b0101;    // 3
                if(adc_0 > SET_DUTY_CYCLE1)     shift_reg_value  = 0b010101;  // 4  
                if(adc_0 > SET_DUTY_CYCLE2)     shift_reg_value  = 0b01010101;// 5  
                if(adc_0 > WRITE_TO_EEPROM)     shift_reg_value  = 0xFFF;     // 6
                Shift_Mode_WriteRegValue( shift_reg_value );
                
                // set period and duty cycle for pwm 1 and 2
                if( (adc_0 > SET_PERIOD_CH1) && (adc_0 < SET_PERIOD_CH2) )
                        if( PWM_CH1_ReadCompare() < (PERIOD_0 + adc_1/8 - 2)) PWM_CH1_WritePeriod( PERIOD_0 + (adc_1/8)  );
                if( (adc_0 > SET_PERIOD_CH2) && (adc_0 < SET_DUTY_CYCLE1) )
                        if( PWM_CH2_ReadCompare() < (PERIOD_0 + adc_1/8 - 2)) PWM_CH2_WritePeriod( PERIOD_0 + (adc_1/8)  );
                
                if( (adc_0 > SET_DUTY_CYCLE1) && (adc_0 < SET_DUTY_CYCLE2) )
                {
                        if( ((adc_1/8)) > 2 && ((adc_1/8) < (PWM_CH1_ReadPeriod()-2))) PWM_CH1_WriteCompare( (adc_1/8)  );
                        else PWM_CH1_WriteCompare( PWM_CH1_ReadPeriod()/2);
                }
                if( (adc_0 > SET_DUTY_CYCLE2) && (adc_0 < WRITE_TO_EEPROM) )
                {
                        if( ((adc_1/8)) > 2 && ((adc_1/8) < (PWM_CH2_ReadPeriod()-2))) PWM_CH2_WriteCompare( (adc_1/8)  );
                        else PWM_CH2_WriteCompare( PWM_CH2_ReadPeriod()/2);
                }
                
                
                sprintf( msg, "ADC 0: %d\t ADC 1: %d\n", adc_0, adc_1);
                UART_1_PutString( msg ); UART_1_PutChar( 13 );
                
                sprintf( msg, "EEPROM 0..7: %d %d %d %d\t %d %d %d %d\n",\
			 EEPROM_ReadByte(0), EEPROM_ReadByte(1), EEPROM_ReadByte(2), EEPROM_ReadByte(3),\
			 EEPROM_ReadByte(4), EEPROM_ReadByte(5), EEPROM_ReadByte(6), EEPROM_ReadByte(7));
                UART_1_PutString( msg ); UART_1_PutChar( 13 );
            }
    } // END read and average ADC values
    
    if(adc_0 > WRITE_TO_EEPROM)
    {
        EEPROM_WriteByte( PWM_CH1_ReadPeriod() >> 8, 0);
        EEPROM_WriteByte( PWM_CH1_ReadPeriod() & 0xFF, 1);

        EEPROM_WriteByte( PWM_CH2_ReadPeriod() >> 8, 2);
        EEPROM_WriteByte( PWM_CH2_ReadPeriod() & 0xFF, 3);

        EEPROM_WriteByte( PWM_CH1_ReadCompare() >> 8, 4);
        EEPROM_WriteByte( PWM_CH1_ReadCompare() & 0xFF, 5);

        EEPROM_WriteByte( PWM_CH2_ReadCompare() >> 8, 6);
        EEPROM_WriteByte( PWM_CH2_ReadCompare() & 0xFF, 7);
    }
    else if( (adc_0 > RUN_FROM_EEPROM) && (adc_0 < SET_PERIOD_CH1) )
    {
        PWM_CH1_WritePeriod( (EEPROM_ReadByte(0) << 8) + EEPROM_ReadByte(1) );
        PWM_CH2_WritePeriod( (EEPROM_ReadByte(2) << 8) + EEPROM_ReadByte(3) );

        PWM_CH1_WriteCompare( (EEPROM_ReadByte(4) << 8) + EEPROM_ReadByte(5) );
        PWM_CH2_WriteCompare( (EEPROM_ReadByte(6) << 8) + EEPROM_ReadByte(7) );
    } // END EEPROM read/write
    
   

   
    
    PWM_SEQU_WritePeriod( RUN_TIME_NORMAL );
}

int main()
{
    CyGlobalIntEnable; /* Enable global interrupts. */

    /* Place your initialization/startup code here (e.g. MyInst_Start()) */
    initialize_hardware();


    for(;;)
    {
        /* Place your application code here. */

        
        
    }
}


void initialize_hardware(void)
{
    UART_1_Start();
    Shift_Mode_Start();
    Shift_Mode_WriteRegValue( 0b0001010101 );
    
    isr_1_StartEx( isr_1_handle );
    
    ADC_Poti_Start();
    ADC_Poti_StartConvert();
    
    PWM_CH1_Start();
    PWM_CH2_Start();
    
    PWM_SEQU_Start();    
    PWM_SEQU_WritePeriod(  RUN_TIME_NORMAL );
    PWM_SEQU_WriteCompare( SEQUENCE_LENGTH );

    EEPROM_Start();
    PWM_CH1_WritePeriod( (EEPROM_ReadByte(0) << 8) + EEPROM_ReadByte(1) );
    PWM_CH2_WritePeriod( (EEPROM_ReadByte(2) << 8) + EEPROM_ReadByte(3) );
    PWM_CH1_WriteCompare( (EEPROM_ReadByte(4) << 8) + EEPROM_ReadByte(5) );
    PWM_CH2_WriteCompare( (EEPROM_ReadByte(6) << 8) + EEPROM_ReadByte(7) );
}

/* [] END OF FILE */
