#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include "tm4c123gh6pm.h"
#include <string.h>
#include <ctype.h>
#include<stdbool.h>

#define RED_LED           (*((volatile uint32_t *)(0x42000000 + (0x400253FC-0x40000000)*32 + 1*4)))
//#define hys               (*((volatile uint32_t *)(0x42000000 + (0x40038E1C-0x40000000)*32 + 1*4)))
#define CS_NOT            (*((volatile uint32_t *)(0x42000000 + (0x400053FC-0x40000000)*32 + 5*4)))
#define transistor        (*((volatile uint32_t *)(0x42000000 + (0x400043FC-0x40000000)*32 + 2*4)))
#define location        (*((volatile uint32_t *)(x)))
#define hib_currentpage        (*((volatile uint32_t *)(0x400FC030)))
#define hib_offset             (*((volatile uint32_t *)(0x400FC034)))

#define MAX_CHARS 80
char uartStr[MAX_CHARS+1];
uint8_t offset[MAX_CHARS],i=0;
char type[MAX_CHARS],str[20];
uint16_t raw,mode_temp=0;
float instantTemp,voltage,V,H,Hys,T,output;
bool h_flag=false, inv_flag=true,h1_flag=true,edge_flag;
uint8_t currentPage,fieldCount=0,arg_Number=0,mode=0,a=0,b=0,d=0,e=0,f=0,hours,minutes,seconds,month,date,cinv=0;
uint16_t space,off_set,hout,vout,N,c=0,year;
uint32_t x,starttime=0,elapsedtime=0,interrupt_rate, abc[13], hib_value;
uint8_t roll=0;
 //#define RED_LED_MASK 2

/*void parseUart0String()
{

    int i;
    uartStr[0]=0;
 for (i=0;i<strlen(uartStr);i++)
 {
     if((uartStr[i]>=32 && uartStr[i]<=44) || (uartStr[i]==47) || (uartStr[i]>=58 && uartStr[i]<=64) || (uartStr[i]>=91 && uartStr[i]<=96) ||(uartStr[i]>=123 && uartStr[i]<=127))
     {
         if ((uartStr[i+1]>=45 && uartStr[i+1]<=46) || (uartStr[i+1]>=48 && uartStr[i+1]<=57) || (uartStr[i+1]>=65 && uartStr[i+1]<=90) || (uartStr[i+1]>=97 && uartStr[i+1]<=122))
                 {
                 offset[fieldCount]=i+1;
                 if (uartStr[i+1]>=48 && uartStr[i+1]<=57)
                 {
                     type[fieldCount]='n';
                 }
                 else
                 {
                     type[fieldCount]='a';
                 }
                 fieldCount++;
                 }

      }
 }
 uint8_t length = strlen(uartStr);
 for (i=0;i<length;i++)
 {
         if((uartStr[i]>=32 && uartStr[i]<=44) || (uartStr[i]==47) || (uartStr[i]>=58 && uartStr[i]<=64) || (uartStr[i]>=91 && uartStr[i]<=96) ||(uartStr[i]>=123 && uartStr[i]<=127))
             {
             uartStr[i]=0;
             }
 }
}*/
void parseUart0String()
{
    uint8_t length = strlen(uartStr);
    int i=0;
    fieldCount=0;
    //uartStr[0]=0;
    if(i==0)
    {
        if ((uartStr[i]>='a' && uartStr[i]<='z'))
             {
                 offset[fieldCount]=i;
                 type[fieldCount]='a';

             }
             else if ((uartStr[i]>=48 && uartStr[i]<=57)||(uartStr[i]>=45 && uartStr[i]<=46))
             {
                 offset[fieldCount]=i;
                 type[fieldCount]='n';
             }
        fieldCount++;

    }
 for (i=0;i<length;i++)
 {

     if((uartStr[i]>=32 && uartStr[i]<=44) || (uartStr[i]==47) || (uartStr[i]>=58 && uartStr[i]<=64) || (uartStr[i]>=91 && uartStr[i]<=96) ||(uartStr[i]>=123 && uartStr[i]<=127))
     {
      continue;
     }
     else if ((uartStr[i]>='a' && uartStr[i]<='z'))
     {
         offset[fieldCount]=i;
         type[fieldCount]='a';

     }
     else if ((uartStr[i]>=48 && uartStr[i]<=57)||(uartStr[i]>=45 && uartStr[i]<=46))
     {
         offset[fieldCount]=i;
         type[fieldCount]='n';
     }
     if(((uartStr[i-1]>=32 && uartStr[i-1]<=44) || (uartStr[i-1]==47) || (uartStr[i-1]>=58 && uartStr[i-1]<=64) || (uartStr[i-1]>=91 && uartStr[i-1]<=96) ||(uartStr[i-1]>=123 && uartStr[i-1]<=127)) && ((uartStr[i]>='a' && uartStr[i]<='z')||(uartStr[i]>=48 && uartStr[i]<=57)||(uartStr[i]>=45 && uartStr[i]<=46)))
     {
         fieldCount++;

     }
 }
 for (i=0;i<length;i++)
  {
          if((uartStr[i]>=32 && uartStr[i]<=44) || (uartStr[i]==47) || (uartStr[i]>=58 && uartStr[i]<=64) || (uartStr[i]>=91 && uartStr[i]<=96) ||(uartStr[i]>=123 && uartStr[i]<=127))
              {
              uartStr[i]=0;
              }
  }
}
bool isCommand( char commandName[20] , int min_Arg)
{
       char *c= &uartStr[offset[0]];
    if(strcmp(commandName,c)==0)
    {
       if( fieldCount>min_Arg)
       {
           return(true);
       }
    }
    return (false);
}
int getValue(int arg_Number)
{
    int value=(atoi(&uartStr[offset[arg_Number+1]]));
    return value;
}
char* getString(int arg_Number)
{
    return &uartStr[offset[arg_Number+1]];
}



// Ap
//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

// Initialize Hardware
void initHw()
{
    // Configure HW to work with 16 MHz XTAL, PLL enabled, sysdivider of 5, creating system clock of 40 MHz
    SYSCTL_RCC_R = SYSCTL_RCC_XTAL_16MHZ | SYSCTL_RCC_OSCSRC_MAIN | SYSCTL_RCC_USESYSDIV | (4 << SYSCTL_RCC_SYSDIV_S);

    // Set GPIO ports to use APB (not needed since default configuration -- for clarity)
    SYSCTL_GPIOHBCTL_R = 0;

    // Enable GPIO port B, C,F,E and A peripherals
    SYSCTL_RCGC2_R = SYSCTL_RCGC2_GPIOA |SYSCTL_RCGC2_GPIOB | SYSCTL_RCGC2_GPIOF | SYSCTL_RCGC2_GPIOE|SYSCTL_RCGC2_GPIOC;

    SYSCTL_RCGCADC_R |= 1;                           // turn on ADC module 0 clocking
    SYSCTL_RCGCUART_R |= SYSCTL_RCGCUART_R0;         // turn-on UART0, leave other UARTs in same status
    SYSCTL_RCGCSSI_R |= SYSCTL_RCGCSSI_R2;           // turn-on SSI2 module
    SYSCTL_RCGCACMP_R|=SYSCTL_RCGCACMP_R0;           //COMPARATOR MODULE 0 ON

    // Configure AN0 as an analog input
        GPIO_PORTE_AFSEL_R |= 0x01;                      // select alternative functions for AN3 (PE0)
        GPIO_PORTE_DEN_R &= ~0x01;                       // turn off digital operation on pin PE0
        GPIO_PORTE_AMSEL_R |= 0x01;                      // turn on analog operation on pin PE0

        // Configure ADC
        ADC0_CC_R = ADC_CC_CS_SYSPLL;                    // select PLL as the time base (not needed, since default value)
        ADC0_ACTSS_R &= ~ADC_ACTSS_ASEN3;                // disable sample sequencer 3 (SS3) for programming
        ADC0_EMUX_R |= ADC_EMUX_EM3_PROCESSOR;            // select SS3 bit in ADCPSSI as trigger
        ADC0_SSMUX3_R |= 3;                               // set first sample to AN3
        ADC0_SSCTL3_R = ADC_SSCTL3_END0;                 // mark first sample as the end
        ADC0_ACTSS_R |= ADC_ACTSS_ASEN3;                 // enable SS3 for operation
        ADC0_SAC_R |= 6;                                //hardware averging



    // Configure LED pin
    GPIO_PORTF_DIR_R |= 2;  // make bit an output
    GPIO_PORTF_DR2R_R |=2; // set drive strength to 2mA (not needed since default configuration -- for clarity)
    GPIO_PORTF_DEN_R |= 2;  // enable LED

    //configure transistor
    GPIO_PORTA_DIR_R |= 4;  // make bit an output
    GPIO_PORTA_DR2R_R |=4; // set drive strength to 2mA (not needed since default configuration -- for clarity)
    GPIO_PORTA_DEN_R |= 4;  // enable LED


    // Configure UART0 pins
    SYSCTL_RCGCUART_R |= SYSCTL_RCGCUART_R0;         // turn-on UART0, leave other uarts in same status
    GPIO_PORTA_DEN_R |= 3;                           // enable digital on UART0 pins: default, added for clarity
    GPIO_PORTA_AFSEL_R |= 3;                         // use peripheral to drive PA0, PA1: default, added for clarity
    GPIO_PORTA_PCTL_R &= 0xFFFFFF00;                 // set fields for PA0 and PA1 to zero
    GPIO_PORTA_PCTL_R |= GPIO_PCTL_PA1_U0TX | GPIO_PCTL_PA0_U0RX;
                                                     // select UART0 to drive pins PA0 and PA1: default, added for clarity

    // Configure UART0 to 115200 baud, 8N1 format (must be 3 clocks from clock enable and config writes)
    UART0_CTL_R = 0;                                 // turn-off UART0 to allow safe programming
    UART0_CC_R = UART_CC_CS_SYSCLK;                  // use system clock (40 MHz)
    UART0_IBRD_R = 21;                               // r = 40 MHz / (Nx115.2kHz), set floor(r)=21, where N=16
    UART0_FBRD_R = 45;                               // round(fract(r)*64)=45
    UART0_LCRH_R = UART_LCRH_WLEN_8 | UART_LCRH_FEN; // configure for 8N1 w/ 16-level FIFO
    UART0_CTL_R = UART_CTL_TXE | UART_CTL_RXE | UART_CTL_UARTEN; // enable TX, RX, and module

    //hibernation module
    SYSCTL_GPIOHBCTL_R=0;
    SYSCTL_RCGCHIB_R|=SYSCTL_RCGCHIB_R0;
    HIB_IM_R|=0x01;
    HIB_CTL_R=0x40|0x01;
    while((HIB_CTL_R & HIB_CTL_WRC)==0)

    /* Configure ~CS
    GPIO_PORTB_DIR_R |=  32;       // make bit 5 output
    GPIO_PORTB_DR2R_R |= 32;      // set drive strength to 2mA
    GPIO_PORTB_DEN_R |=  32;       // enable bit 5 for digital*/

    // Configure SSI2 pins for SPI configuration
    GPIO_PORTB_DIR_R |= 0xB0;                        // make bits 4 and 7 outputs
    GPIO_PORTB_DR2R_R |= 0xB0;                       // set drive strength to 2mA
    GPIO_PORTB_AFSEL_R |= 0xB0;                      // select alternative functions for MOSI, SCLK pins
    GPIO_PORTB_PCTL_R |= GPIO_PCTL_PB7_SSI2TX | GPIO_PCTL_PB4_SSI2CLK | GPIO_PCTL_PB5_SSI2FSS; // map alt fns to SSI2
    GPIO_PORTB_DEN_R |= 0xB0;                        // enable digital operation on TX, CLK pins
    GPIO_PORTB_PUR_R |= 0x10;                        // must be enabled when SPO=1


    //Configure the SSI2 as a SPI master, mode 3, 8bit operation, 1 MHz bit rate

   SSI2_CR1_R &= ~SSI_CR1_SSE;                      // turn off SSI2 to allow re-configuration
   SSI2_CR1_R = 0;                                  // select master mode
   SSI2_CC_R = 0;                                   // select system clock as the clock source
   SSI2_CPSR_R = 40;                                // set bit rate to 1 MHz (if SR=0 in CR0)
   SSI2_CR0_R = SSI_CR0_SPH | SSI_CR0_SPO | SSI_CR0_FRF_MOTO | SSI_CR0_DSS_16; // set SR=0, mode 3 (SPH=1, SPO=1), 16-bit
   SSI2_CR1_R |= SSI_CR1_SSE;                       // turn on SSI2


    // Configure Timer 1
       //SYSCTL_RCGCTIMER_R |= SYSCTL_RCGCTIMER_R1;       // turn-on timer
       /*TIMER1_CTL_R &= ~TIMER_CTL_TAEN;                 // turn-off timer before reconfiguring
       TIMER1_CFG_R = TIMER_CFG_32_BIT_TIMER;           // configure as 32-bit timer (A+B)
       TIMER1_TAMR_R = mode;                             // configure for periodic mode (count down)
       TIMER1_TAILR_R = interrupt_rate;                        // set load value to 2e5 for 200 Hz interrupt rate
       TIMER1_IMR_R = TIMER_IMR_TATOIM;                 // turn-on interrupts
       NVIC_EN0_R |= 1 << (INT_TIMER1A-16);             // turn-on interrupt 37 (TIMER1A)
       TIMER1_CTL_R |= TIMER_CTL_TAEN;                  // turn-on timer*/

   // Configure Comaparator pins for Comparator configuration
   //GPIO_PORTC_LOCK_R=0x4C4F434B;
   //GPIO_PORTC_CR_R |= 0xC0;
   GPIO_PORTC_DIR_R &= ~0xC0;                        // make bits 6 and 7 inputs
   GPIO_PORTC_DEN_R &= ~0xC0;                        //disable digital operation
   GPIO_PORTC_AFSEL_R |= 0xC0;                      //turn on auxilary fucntion of pin PC6 and PC7
   GPIO_PORTC_AMSEL_R=0xC0;

//Configure EEPROM
SYSCTL_RCGCEEPROM_R=1;
__asm (" NOP");
__asm (" NOP");
__asm (" NOP");
__asm (" NOP");
__asm (" NOP");
__asm (" NOP");
while(EEPROM_EEDONE_R & 1);
 if ((EEPROM_EESUPP_R & 4==1) |(EEPROM_EESUPP_R & 8==1) )
{
     check:   SYSCTL_SREEPROM_R=1;
    SYSCTL_SREEPROM_R=0;
    while(EEPROM_EEDONE_R & 1);
    if ((EEPROM_EESUPP_R & 4==1) |(EEPROM_EESUPP_R & 8==1) )
    {
        goto check;
    }
    SYSCTL_SREEPROM_R=1;
    __asm (" NOP");
    __asm (" NOP");
    __asm (" NOP");
    __asm (" NOP");
    __asm (" NOP");
    __asm (" NOP");
    while(EEPROM_EEDONE_R & 1);
     if ((EEPROM_EESUPP_R & 4==1) |(EEPROM_EESUPP_R & 8==1) )
    {
         check1:   SYSCTL_SREEPROM_R=1;
        SYSCTL_SREEPROM_R=0;
        while(EEPROM_EEDONE_R & 1);
        if ((EEPROM_EESUPP_R & 4==1) |(EEPROM_EESUPP_R & 8==1) )
        {
            goto check1;
        }
    }
}


}


int16_t readAdc0Ss3()
{
    ADC0_PSSI_R |= ADC_PSSI_SS3;                     // set start bit
    while (ADC0_ACTSS_R & ADC_ACTSS_BUSY);           // wait until SS3 is not busy
    return ADC0_SSFIFO3_R;                           // get single result from the FIFO

}

// Blocking function that writes a serial character when the UART buffer is not full
void putcUart0(char c)
{
    while (UART0_FR_R & UART_FR_TXFF);               // wait if uart0 tx fifo full
    UART0_DR_R = c;                                  // write character to fifo
}

// Blocking function that writes a string when the UART buffer is not full
void putsUart0(char* str)
{
    uint8_t i;
    for (i = 0; i < strlen(str); i++)
      putcUart0(str[i]);
}


// Blocking function that returns with serial data once the buffer is not empty
char getcUart0()
{
    while (UART0_FR_R & UART_FR_RXFE);               // wait if uart0 rx fifo empty
    return UART0_DR_R & 0xFF;                        // get character from fifo
}


void getUart0string()
{
    uint8_t count=0;
      while(count<MAX_CHARS+1)
      {
      char c=getcUart0();
                if (c==8)
                {
                    if (count>0)
                    {
                        count--;
                    }
                    continue;
                }
                if (c==13)
                {
                    uartStr[count] = 0;
                    break;
                }
                if (c>=32)
                {
                    uartStr[count++]=tolower(c);
                    if (count==MAX_CHARS)
                    {
                        uartStr[count]=0;
                        break;
                    }
                }
      }
}
bool leapyear(int y)
{
    if((y%400==0)||((y%4==0)&&(y%100!=0)))
    {
        return true;
    }
    else
        return false;
}

void set_Time()
{
     a=month;
     b=date;
     c=year;
     d=hours;
     e=minutes;
     f=seconds;
    int t=elapsedtime-starttime;
     //int t=100000;
    while(t!=0)
    {
        if(t>31622400)//31622400 seconds in a leap year
        {
            if (leapyear(c)==1)
            {
                t=t-31622400;
                c++;
                continue;
            }
        }
        else if(t>31536000)//31536000 seconds in a year
        {
            t=t-31536000;
            c++;
            continue;
        }
        switch (a)
                {
                case 1://jan, 2678400 seconds in 31 days
                    if(t>2678400)
                    {
                        t=t-2678400;
                        a++;
                        continue;
                    }
                    break;
                case 2://feb 2505600 seconds for 29daysi.e,leap year; 2419200 seconds for a 28days
                    if(t>2505600)
                    {
                        if(leapyear(c)==1)
                        {
                            t=t-2505600;
                            a++;
                            continue;
                        }
                    }
                    else if(t>2419200)
                    {
                        t=t-2419200;
                        a++;
                        continue;
                    }
                    break;
                case 3://march, 2678400 seconds in 31 days
                            if(t>2678400)
                            {
                                t=t-2678400;
                                a++;
                                continue;
                            }
                            break;
                case 4://april, 2592000 seconds in 30 days
                            if(t>2678400)
                            {
                                t=t-2592000;
                                a++;
                                continue;
                            }
                            break;
                case 5://MAY, 2678400 seconds in 31 days
                            if(t>2678400)
                            {
                                t=t-2678400;
                                a++;
                                continue;
                            }
                            break;
                case 6://june, 2592000 seconds in 30 days
                                    if(t>2678400)
                                    {
                                        t=t-2592000;
                                        a++;
                                        continue;
                                    }
                                    break;
                case 7://July, 2678400 seconds in 31 days
                                    if(t>2678400)
                                    {
                                        t=t-2678400;
                                        a++;
                                        continue;
                                    }
                                    break;
                case 8://august, 2678400 seconds in 31 days
                                    if(t>2678400)
                                    {
                                        t=t-2678400;
                                        a++;
                                        continue;
                                    }
                                    break;
                case 9://september, 2592000 seconds in 30 days
                                            if(t>2678400)
                                            {
                                                t=t-2592000;
                                                a++;
                                                continue;
                                            }
                                            break;
                case 10://october, 2678400 seconds in 31 days
                                            if(t>2678400)
                                            {
                                                t=t-2678400;
                                                a++;
                                                continue;
                                            }
                                            break;
                case 11://november, 2592000 seconds in 30 days
                                                    if(t>2678400)
                                                    {
                                                        t=t-2592000;
                                                        a++;
                                                        continue;
                                                    }
                                                    break;
                case 12://december, 2678400 seconds in 31 days
                                                    if(t>2678400)
                                                    {
                                                        t=t-2678400;
                                                        a=1;
                                                        c++;
                                                        continue;
                                                    }
                                                    break;
                }
        if(t>86400)//86400seconds in a day
                {
                    if((a==1||a==3||a==5||a==7||a==8||a==10||a==12)&&(b<31))
                    {
                        t=t-86400;
                        b++;
                        continue;
                    }
                    else if((a==1||a==3||a==5||a==7||a==8||a==10)&&(b==31))
                                {
                                    t=t-86400;
                                    b=1;
                                    a++;
                                    continue;
                                }
                    else if((a==12)&&(b==31))
                    {
                        t=t-86400;
                        b=1;
                        a=1;
                        c++;
                        continue;
                    }
                    if((a==4||a==6||a==9||a==11)&&(b<30))
                    {
                        t=t-86400;
                        b++;
                        continue;
                    }
                    if((a==4||a==6||a==9||a==11)&&(b==30))
                    {
                        t=t-86400;
                        b=1;
                        a++;
                        continue;
                    }
                    if(a==2)
                    {
                        if(leapyear(c)==1)
                        {
                            if(b<29)
                            {
                            t=t-86400;
                            b++;
                            continue;
                            }
                            if(b==29)
                            {
                                t=t-86400;
                                b=1;
                                a++;
                                continue;
                            }
                        }
                    else if (b<28)
                    {
                        t=t-86400;
                        b++;
                        continue;
                    }
                    else if(b==28)
                    {
                        t=t-86400;
                        b=1;
                        a++;
                        continue;
                    }
                  }
          }
        if(t>3600)//3600seconds per hour
                {
                    if(d<23)
                    {
                        t=t-3600;
                        d++;
                        continue;
                    }
                    else if (d==23)
                    {
                        if((a==1||a==3||a==5||a==7||a==8||a==10))
                        {
                            if(b<31)
                            {
                                t=t-3600;
                                d=0;
                                b++;
                                continue;
                            }
                            if(b==31)
                            {
                                t=t-3600;
                                d=0;
                                b=1;
                                a++;
                                continue;
                            }
                        }
                        if (a==12)
                        {
                            if(b<31)
                            {
                                t=t-3600;
                                d=0;
                                b++;
                                continue;
                            }
                            if(b==31)
                            {
                                t=t-3600;
                                d=0;
                                b=1;
                                a=1;
                                c++;
                            }
                        }
                        if((a==4||a==6||a==9||a==11))
                        {
                            if(b<30)
                            {
                                t=t-3600;
                                d=0;
                                b++;
                                continue;
                            }
                            if(b==30)
                            {
                                t=t-3600;
                                d=0;
                                b=1;
                                a++;
                                continue;
                            }
                        }
                        if (a==2)
                        {
                            if(leapyear(c)==1)
                            {
                                if(b<29)
                                {
                                    t=t-3600;
                                    d=0;
                                    b++;
                                    continue;
                                }
                                else if(b==29)
                                {
                                    t=t-3600;
                                    d=0;
                                    b=1;
                                    a++;
                                    continue;
                                }
                            }
                            else
                            {
                                if(b<28)
                                {
                                    t=t-3600;
                                    d=0;
                                    b++;
                                    continue;
                                }
                                else if(b==28)
                                {
                                    t=t-3600;
                                    d=0;
                                    b=1;
                                    a++;
                                    continue;
                                }
                            }
                        }
                        }
                        }
        if(t>60)//for minutes
        {
            if(e<59)
            {
                t=t-60;
                e++;
                continue;
            }
            if(e==59)
            {
             if(d<23)
             {
              e=0;
              d++;
              t=t-60;
              continue;
             }
             else if(d==23)
             {
                 if((a==1||a==3||a==5||a==7||a==8||a==10))//for months ending with 31days
                 {
                     if(b<31)
                     {
                         b++;
                         d=0;
                         e=0;
                         t=t-60;
                         continue;
                     }

                     else if(b==31)
                     {
                         b=1;
                         d=0;
                         e=0;
                         a++;
                         t=t-60;
                         continue;
                     }
                 }
                 if((a==4||a==6||a==9||a==11))//for months having 30days
                 {
                     if(b<30)
                     {
                         b++;
                         d=0;
                         e=0;
                         t=t-60;
                         continue;
                     }

                     else if(b==30)
                     {
                         b=1;
                         d=0;
                         e=0;
                         a++;
                         t=t-60;
                         continue;
                     }
                 }
                 if(a==12)//december
                 {
                     if(b<31)
                     {
                         b++;
                         d=0;
                         e=0;
                         t=t-60;
                         continue;
                     }

                     else if(b==31)
                     {
                         b=1;
                         d=0;
                         e=0;
                         a=1;
                         c++;
                         t=t-60;
                         continue;
                     }
                     }
                 if (a==2)
                 {
                     if (leapyear(c)==1)//if leap year
                     {
                             if(b<29)
                             {
                                 t=t-60;
                                 d=0;
                                 e=0;
                                 b++;
                                 continue;
                             }
                             else if(b==29)
                             {
                                 t=t-60;
                                 d=0;
                                 b=1;
                                 e=0;
                                 a++;
                                 continue;
                             }
                         }
                     else
                     {
                         if(b<28)
                         {
                             t=t-60;
                             d=0;
                             e=0;
                             b++;
                             continue;
                         }
                         else if(b==28)
                         {
                             t=t-60;
                             d=0;
                             e=0;
                             b=1;
                             a++;
                             continue;
                         }
                     }


                 }
                 }
                }
               }
        if(t>0)//for seconds
        {
            if(f<59)
            {
                t=t-1;
                f++;
                continue;
            }
            if(f==59)
            {
                if(e<59)
                {
                    f=0;
                    e++;
                    t=t-1;
                    continue;
                }
                else if(e==59)
                {
                    if(d<23)
                    {
                        t=t-1;
                        d++;
                        e=0;
                        f=0;
                        continue;
                    }
                    else if(d==23)
                    {
                        if((a==1||a==3||a==5||a==7||a==8||a==10))//for months ending with 31days
                        {
                            if(b<31)
                            {
                                b++;
                                d=0;
                                e=0;
                                f=0;
                                t=t-1;
                                continue;
                            }

                            else if(b==31)
                            {
                                b=1;
                                d=0;
                                e=0;
                                f=0;
                                a++;
                                t=t-1;
                                continue;
                            }
                        }
                        if((a==4||a==6||a==9||a==11))//for months having 30days
                        {
                            if(b<30)
                            {
                                b++;
                                d=0;
                                e=0;
                                f=0;
                                t=t-1;
                                continue;
                            }

                            else if(b==30)
                            {
                                b=1;
                                d=0;
                                e=0;
                                f=0;
                                a++;
                                t=t-1;
                                continue;
                            }
                        }
                        if(a==12)//december
                        {
                            if(b<31)
                            {
                                b++;
                                d=0;
                                e=0;
                                f=0;
                                t=t-1;
                                continue;
                            }

                            else if(b==31)
                            {
                                b=1;
                                d=0;
                                e=0;
                                a=1;
                                f=0;
                                c++;
                                t=t-1;
                                continue;
                            }
                            }
                        if (a==2)
                        {
                            if (leapyear(c)==1)//if leap year
                            {
                                    if(b<29)
                                    {
                                        t=t-1;
                                        d=0;
                                        f=0;
                                        e=0;
                                        b++;
                                        continue;
                                    }
                                    else if(b==29)
                                    {
                                        t=t-1;
                                        f=0;
                                        d=0;
                                        b=1;
                                        e=0;
                                        a++;
                                        continue;
                                    }
                                }
                            else
                            {
                                if(b<28)
                                {
                                    t=t-1;
                                    f=0;
                                    d=0;
                                    e=0;
                                    b++;
                                    continue;
                                }
                                else if(b==28)
                                {
                                    t=t-1;
                                    f=0;
                                    d=0;
                                    e=0;
                                    b=1;
                                    a++;
                                    continue;
                                }
                            }


                        }
                        }
                }
            }
        }
    }
}










//proximate busy waiting (in units of microseconds), given a 40 MHz system clock
void waitMicrosecond(uint32_t us)
{
    __asm("WMS_LOOP0:   MOV  R1, #6");          // 1
    __asm("WMS_LOOP1:   SUB  R1, #1");          // 6
    __asm("             CBZ  R1, WMS_DONE1");   // 5+1*3
    __asm("             NOP");                  // 5
    __asm("             NOP");                  // 5
    __asm("             B    WMS_LOOP1");       // 5*2 (speculative, so P=1)
    __asm("WMS_DONE1:   SUB  R0, #1");          // 1
    __asm("             CBZ  R0, WMS_DONE0");   // 1
    __asm("             NOP");                  // 1
    __asm("             B    WMS_LOOP0");       // 1*2 (speculative, so P=1)
    __asm("WMS_DONE0:");                        // ---
                                                // 40 clocks/us + error
}

uint8_t get_Firstpage()
{
    currentPage=128;
    hib_currentpage=currentPage;
    while(!(HIB_CTL_R & HIB_CTL_WRC));
    return currentPage;
}
uint8_t get_Nextpage()
{
    currentPage=hib_currentpage;
    while(!(HIB_CTL_R & HIB_CTL_WRC));
    currentPage++;
    hib_currentpage=currentPage;
    while(!(HIB_CTL_R & HIB_CTL_WRC));
    return currentPage;
}
void memory_calculation()
{
    if (mode==1)
    {
        space=N*64;
    }
    else if (mode==2)
    {
        space=32+(N*32);
    }
}

void erase_data()
{
    memory_calculation();
    //space=space/8;
    uint8_t  (pages)= ceil (((space/1024/8)));
    get_Firstpage();
    for (i=0;i<pages;i++)
    {
        FLASH_FMA_R=currentPage*1024;
        FLASH_FMC_R=0xA4420000|2;
        while(FLASH_FMC_R & 2);
        currentPage=get_Nextpage();
    }

}
uint32_t get_Nextaddress()
{
    off_set=hib_offset;
    while(!(HIB_CTL_R & HIB_CTL_WRC));
  if(off_set==1024)
  {

      off_set=0;
      hib_offset=off_set;
      while(!(HIB_CTL_R & HIB_CTL_WRC));
      currentPage=get_Nextpage();
  }
  hib_currentpage=currentPage;
  x= (currentPage*1024)+off_set;
  off_set++;
  hib_offset=off_set;
  while(!(HIB_CTL_R & HIB_CTL_WRC));
  return x;
}
uint32_t get_firstaddress()
{
    currentPage=get_Firstpage();
    off_set=0;
    hib_offset=off_set;
    while(!(HIB_CTL_R & HIB_CTL_WRC));
    x= (currentPage*1024)+off_set;
    off_set++;
    hib_offset=off_set;
    while(!(HIB_CTL_R & HIB_CTL_WRC));
    return x;
}
void timer1ISR()
{
    if(N!=0)
    {
        if(mode_temp==0)
        {
        raw = readAdc0Ss3();
        voltage = ((3.3 *raw) /4096);
        sprintf(str, "voltage (v): %3.1f\r\n", voltage);
        putsUart0(str);
        N--;
        {
            FLASH_FMA_R= get_Nextaddress();
            FLASH_FMD_R=voltage*100;
                FLASH_FMC_R=0xA4420000|1;
                while(FLASH_FMC_R & 1);
            get_Nextaddress();
            get_Nextaddress();
            get_Nextaddress();
           // get_Nextaddress();
        }

        }
        else if(mode_temp==8)
        {
            raw = readAdc0Ss3();
            instantTemp = 147.5-((75*3.3 *raw) /4096);
           sprintf(str, "temperature (C): %3.1f\r\n", instantTemp);
            putsUart0(str);
            N--;
            {
                FLASH_FMA_R= get_Nextaddress();
                FLASH_FMD_R=instantTemp*100;
                    FLASH_FMC_R=0xA4420000|1;
                    while(FLASH_FMC_R & 1);
                get_Nextaddress();
                get_Nextaddress();
                get_Nextaddress();
              //  get_Nextaddress();
            }
        }
    }
    else
    {
        TIMER1_CTL_R &= ~(TIMER_CTL_TAEN);             // turn-off timer
    }
    TIMER1_ICR_R = TIMER_ICR_TATOCINT;

}

void EEpromlocation(uint8_t block,uint8_t offset)
{
while(EEPROM_EEDONE_R & 1);
EEPROM_EEBLOCK_R=block;
EEPROM_EEOFFSET_R=offset;
}

void hibernateISR()
    {
        if(N!=0)
        {
            if(mode_temp==0)
            {
            raw = readAdc0Ss3();
            voltage = ((3.3 *raw) /4096);
            sprintf(str, "voltage (v): %3.1f\r\n", voltage);
            putsUart0(str);
            N--;
            {
                FLASH_FMA_R= get_Nextaddress();
                FLASH_FMD_R=voltage*100;
                FLASH_FMC_R=0xA4420000|1;
                while(FLASH_FMC_R & 1);
                get_Nextaddress();
                get_Nextaddress();
                get_Nextaddress();
                //get_Nextaddress();
                EEpromlocation(15,0);
                if (EEPROM_EERDWR_R==1)
                {
                    HIB_CTL_R|=HIB_CTL_RTCWEN|HIB_CTL_PINWEN;
                    while(!(HIB_CTL_R & HIB_CTL_WRC));
                    HIB_CTL_R|= HIB_CTL_HIBREQ;
                    while(!(HIB_CTL_R & HIB_CTL_WRC));
                }

            }
            }
            else if(mode_temp==8)
            {
                raw = readAdc0Ss3();
                instantTemp = 147.5-((75*3.3 *raw) /4096);
                sprintf(str, "temperature (C): %3.1f\r\n", instantTemp);
                putsUart0(str);
                N--;
                {
                    FLASH_FMA_R= get_Nextaddress();
                    FLASH_FMD_R=instantTemp*100;
                    FLASH_FMC_R=0xA4420000|1;
                        while(FLASH_FMC_R & 1);
                    get_Nextaddress();
                    get_Nextaddress();
                    get_Nextaddress();
                    //get_Nextaddress();
                    EEpromlocation(15,0);
                    if (EEPROM_EERDWR_R==1)
                    {
                        HIB_CTL_R|=HIB_CTL_RTCWEN|HIB_CTL_PINWEN;
                        while(!(HIB_CTL_R & HIB_CTL_WRC));
                        HIB_CTL_R|= HIB_CTL_HIBREQ;
                        while(!(HIB_CTL_R & HIB_CTL_WRC));
                    }
                }
            }
            HIB_RTCM0_R = T + HIB_RTCC_R;
            while(!(HIB_CTL_R & HIB_CTL_WRC));
            /*HIB_IM_R &= ~ HIB_IM_WC;
            HIB_IC_R = HIB_IC_RTCALT0;
            while(!(HIB_CTL_R & HIB_CTL_WRC));
            NVIC_EN1_R |= 1<<(INT_HIBERNATE-16-32);
            HIB_IC_R = HIB_IC_RTCALT0;
            while(!(HIB_CTL_R & HIB_CTL_WRC));
            HIB_IM_R |= HIB_IM_RTCALT0;*/
        }
        else
        {
            HIB_IM_R &= ~ HIB_IM_RTCALT0;
        }
        HIB_IC_R = HIB_IC_RTCALT0;
        //HIB_RTCM0_R = T + HIB_RTCC_R;
    }



void comparator0ISR()
   {
    if(h_flag == false)
    {
    if(inv_flag == true )
    {
    if(N!=0)
    {
        raw = readAdc0Ss3();
        voltage = ((3.3 *raw) /4096);
        sprintf(str, "voltage (v): %3.5f\r\n", voltage);
        putsUart0(str);
        N--;
        {
        {
            FLASH_FMA_R=x;
            FLASH_FMD_R=voltage*100;
            FLASH_FMC_R=0xA4420000|1;
            while(FLASH_FMC_R & 1);
            get_Nextaddress();
            get_Nextaddress();
            get_Nextaddress();
            get_Nextaddress();
        }
        {
            FLASH_FMA_R=x;
            FLASH_FMD_R=HIB_RTCC_R;
            FLASH_FMC_R=0xA4420000|1;
            while(FLASH_FMC_R & 1);
            get_Nextaddress();
            get_Nextaddress();
            get_Nextaddress();
            get_Nextaddress();
        }
        }
        //cinv=2;
        COMP_ACCTL0_R ^=COMP_ACCTL1_CINV ;
        waitMicrosecond(100000);
        COMP_ACMIS_R=1;
        inv_flag=false;
    }
    else
    {
        COMP_ACINTEN_R &= ~1;

    }
    }
    else if(inv_flag==false)
    {
        //cinv=0;
        COMP_ACCTL0_R ^= COMP_ACCTL1_CINV ;
        waitMicrosecond(100000);
        COMP_ACMIS_R=1;
        inv_flag=true;
        }
    waitMicrosecond(1000);
    COMP_ACMIS_R=1;
    }
    else if (h_flag)
    {
        if((h1_flag==1)&&(cinv==0))
        {
        if(N!=0)
        {
            raw = readAdc0Ss3();
            voltage = ((3.3 *raw) /4096);
            raw = readAdc0Ss3();
            voltage = ((3.3 *raw) /4096);
            sprintf(str, "voltage (v): %3.1f\r\n", voltage);
            putsUart0(str);
            N--;
            {
            {
                FLASH_FMD_R=voltage*100;
                FLASH_FMA_R=x;
                FLASH_FMC_R=0xA4420000|1;
                while(FLASH_FMC_R & 1);
                get_Nextaddress();
                get_Nextaddress();
                get_Nextaddress();
                get_Nextaddress();
            }
            {
                FLASH_FMD_R=HIB_RTCC_R;
                FLASH_FMA_R=x;
                FLASH_FMC_R=0xA4420000|1;
                while(FLASH_FMC_R & 1);
                get_Nextaddress();
                get_Nextaddress();
                get_Nextaddress();
                get_Nextaddress();
            }
            }
            SSI2_DR_R=hout;
            while(SSI2_SR_R & SSI_SR_BSY);
            h1_flag=false;
            waitMicrosecond(10000);
        }

        else
        {
            COMP_ACINTEN_R &= ~1;
            h1_flag=0;
            cinv=0;
            SSI2_DR_R=vout;
            waitMicrosecond(10000);
            h_flag=0;
        }
        }
        else if((h1_flag==0)&&(cinv==0))
        {
            COMP_ACCTL0_R ^=COMP_ACCTL1_CINV ;
            waitMicrosecond(100000);
            COMP_ACMIS_R=1;
            //waithalfMicrosecond(1000000);
            cinv=1;
            SSI2_DR_R=vout;
           // waithalfMicrosecond(1000000);
        }
        else if ((h1_flag==0)&&(cinv==1))
        {
            COMP_ACCTL0_R ^= COMP_ACCTL1_CINV ;
            waitMicrosecond(100000);
            COMP_ACMIS_R=1;
           // waithalfMicrosecond(1000000);
            cinv=0;
            h1_flag=true;
            //waithalfMicrosecond(1000000);
        }
        waitMicrosecond(100000);
        COMP_ACMIS_R=1;

    }
    }

/*
void comparator0ISR()
{
    if(h_flag == false)
    {
       // raw = readAdc0Ss3();
        //voltage = ((3.3 *raw) /4096);
    //if (voltage > V)
    //{
    if(N!=0)
    {
        raw = readAdc0Ss3();

        sprintf(str, "voltage (v): %3.1f\r\n", voltage);
        putsUart0(str);
        N--;
    }
   // }
        else
        {
            COMP_ACINTEN_R &= ~1;
        }
        COMP_ACMIS_R=1;
        waitMicrosecond(10000);
    }
    else if(h_flag == true)
    {
       if(h1_flag==1)
       {
        if(N!=0)
            {
                raw = readAdc0Ss3();
                voltage = ((3.3 *raw) /4096);
                sprintf(str, "voltage (v): %3.1f\r\n", voltage);
                putsUart0(str);
                N--;
                h1_flag=false;
                SSI2_DR_R=hout;
            }
                else
                {
                    COMP_ACINTEN_R &= ~1;
                }
                COMP_ACMIS_R=1;
                waitMicrosecond(2000);
       }
       if(h1_flag==0)
       {
           SSI2_DR_R=vout;
           h1_flag=false;
           COMP_ACMIS_R=1;
       }

    }

}

void comparator0ISR()
{
    {
        if(h_flag == false)
        {
            if(N!=0)
                {
                    raw = readAdc0Ss3();
                    voltage = ((3.3 *raw) /4096);
                    sprintf(str, "voltage (v): %3.5f\r\n", voltage);
                    putsUart0(str);
                    N--;
                    waitMicrosecond(1000);
                    COMP_ACMIS_R  = COMP_ACMIS_IN0;

        }
            else
                   {
                       COMP_ACINTEN_R &= ~1;
                   }
            waitMicrosecond(1000);
            COMP_ACMIS_R  = COMP_ACMIS_IN0;
            waitMicrosecond(1000);
        }
}
}*/



int main(void)
{


    char str[10];
    // Initialize hardware
    initHw();
    RED_LED = 1;
    waitMicrosecond(500000);
    RED_LED = 0;
    waitMicrosecond(500000);

    // Toggle red LED  second

while(1)
{
    fieldCount=0;
    getUart0string();
    parseUart0String();
if(isCommand("reset",0))
{
    NVIC_APINT_R = 0x05FA0000|0x04|0x01;//writing 0x05FA to vectkey and setting bits system reset request and vectreset high.
    putsUart0("\r\n");
}
if(isCommand("temp",0))
{
    raw = readAdc0Ss3();
    instantTemp = 147.5-((75*3.3 *raw) /4096);
    sprintf(str, "temperature (C): %3.1f\r\n", instantTemp);
            putsUart0(str);
}
if(isCommand("voltage",0))
{
    raw = readAdc0Ss3();
    voltage = ((3.3 *raw) /4096);
    sprintf(str, "voltage (v): %3.1f\r\n", voltage);
            putsUart0(str);
}
if(isCommand("input",1))
{
   if(strcmp("voltage",getString(0))==0)
    {
       mode_temp=0;
       EEpromlocation(12,0);
       EEPROM_EERDWR_R=0;
        ADC0_SSCTL3_R = ADC_SSCTL3_END0| mode_temp;
        putsUart0("\r\n");

    }
    else if(strcmp("temp",getString(0))==0)
    {
        EEpromlocation(12,0);
        EEPROM_EERDWR_R=1;
        mode_temp=8;
        ADC0_SSCTL3_R = ADC_SSCTL3_END0|mode_temp;
        putsUart0("\r\n");
    }
}
if (isCommand("samples",1))
   {
    N=getValue(0);
    EEpromlocation(7,0);
    EEPROM_EERDWR_R=N;
    putsUart0("\r\n");
   }


if (isCommand("threshold",1))
{
    transistor=1;
    V=atof(getString(0));
     uint16_t D=round((V*4096)/2.048);
     vout=0x3000 | D;

         SSI2_DR_R = vout;                  // write data
         putsUart0("\r\n");

}
if (isCommand("hystersis",1))
{
    H=atof(getString(0));
    if(edge_flag)
    {
         Hys=V+H;
    }
    else if(!edge_flag)
    {
        Hys=V-H;
    }
    h_flag=true;
    transistor=1;
     uint16_t D1=round((Hys*4096)/2.048);
    hout=0x3000 | D1;
     h_flag=true;
     putsUart0("\r\n");
}
if(isCommand("edge",1))
{

    if(strcmp("positive",getString(0))==0)
    {
        edge_flag=true;
        EEpromlocation(10,0);
        EEPROM_EERDWR_R=1;
        //cinv=0;
        //COMP_ACCTL0_R|= COMP_ACCTL1_ISEN_RISE;
        COMP_ACCTL0_R &= ~COMP_ACCTL1_CINV;
        waitMicrosecond(1000);
        COMP_ACMIS_R  = COMP_ACMIS_IN0;

        putsUart0("\r\n");
    }
    else if(strcmp("negetive",getString(0))==0)
    {
        edge_flag=false;
        EEpromlocation(10,0);
        EEPROM_EERDWR_R=0;
        //cinv=2;
        //COMP_ACCTL0_R|= COMP_ACCTL1_ISEN_RISE;
        COMP_ACCTL0_R |= COMP_ACCTL1_CINV;
        waitMicrosecond(1000);
        COMP_ACMIS_R  = COMP_ACMIS_IN0;
        putsUart0("\r\n");
    }
}
if(isCommand("sleep",1))
{

    if(strcmp("on",getString(0))==0)
    {
        transistor=0;
        EEpromlocation(15,0);
        EEPROM_EERDWR_R=1;
        HIB_CTL_R|=HIB_CTL_RTCWEN|HIB_CTL_PINWEN;
        while(!(HIB_CTL_R & HIB_CTL_WRC));
        HIB_CTL_R|= HIB_CTL_HIBREQ;
        while(!(HIB_CTL_R & HIB_CTL_WRC));
    }
    else if(strcmp("off",getString(0))==0)
    {
        EEpromlocation(15,0);
        EEPROM_EERDWR_R=0;
    }
}

if(isCommand("leveling",1))
{

    if(strcmp("on",getString(0))==0)
    {
        EEpromlocation(11,0);
        EEPROM_EERDWR_R=1;
    }
    else if(strcmp("off",getString(0))==0)
    {
        EEpromlocation(11,0);
        EEPROM_EERDWR_R=0;
    }
}
if (isCommand("periodic",1))
{
    mode=2;
    EEpromlocation(8,0);
    EEPROM_EERDWR_R=mode;
    EEpromlocation(15,0);
    EEPROM_EERDWR_R=0;
    erase_data();
    get_firstaddress();
    {
                FLASH_FMA_R=x;
                FLASH_FMD_R=HIB_RTCC_R;
                FLASH_FMC_R=0xA4420000|1;
                while(FLASH_FMC_R & 1);
                get_Nextaddress();
                get_Nextaddress();
                get_Nextaddress();
                //get_Nextaddress();
            }
    T=atof(getString(0));
    EEpromlocation(9,0);
    EEPROM_EERDWR_R=T;
    transistor=0;
    if(T<=3)
    {
    interrupt_rate=T*40000000;


    {
           SYSCTL_RCGCTIMER_R |= SYSCTL_RCGCTIMER_R1;       // turn-on timer
           TIMER1_CTL_R &= ~TIMER_CTL_TAEN;                 // turn-off timer before reconfiguring
           TIMER1_CFG_R = TIMER_CFG_32_BIT_TIMER;           // configure as 32-bit timer (A+B)
           TIMER1_TAMR_R = mode;                             // configure for periodic mode (count down)
           TIMER1_TAILR_R = interrupt_rate;                        // set load value to 2e5 for 200 Hz interrupt rate
           TIMER1_IMR_R = TIMER_IMR_TATOIM;                 // turn-on interrupts
           NVIC_EN0_R |= 1 << (INT_TIMER1A-16);             // turn-on interrupt 37 (TIMER1A)
           TIMER1_CTL_R |= TIMER_CTL_TAEN;                  // turn-on timer
    }
    }
    else
    {
            HIB_RTCM0_R = T + HIB_RTCC_R;
            HIB_IM_R &= ~ HIB_IM_WC;
            HIB_IC_R = HIB_IC_RTCALT0;
            while(!(HIB_CTL_R & HIB_CTL_WRC));
            NVIC_EN1_R |= 1<<(INT_HIBERNATE-16-32);
            HIB_IC_R = HIB_IC_RTCALT0;
            while(!(HIB_CTL_R & HIB_CTL_WRC));
            HIB_IM_R |= HIB_IM_RTCALT0;
    }
    putsUart0("\r\n");
    }



if (isCommand("trigger",0))
{
    erase_data();
    get_firstaddress();
    mode=1;
    EEpromlocation(8,0);
    EEPROM_EERDWR_R=mode;
    COMP_ACMIS_R  |= COMP_ACMIS_IN0;
    COMP_ACCTL0_R |=COMP_ACCTL0_ASRCP_PIN0|COMP_ACCTL0_TSEN_LEVEL ;//| cinv;// | COMP_ACCTL0_ISLVAL ;   COMP_ACCTL0_ISEN_RISE              //CONFIGURE COMPARATOR
    COMP_ACMIS_R |=1;
    waitMicrosecond(1000);
    NVIC_EN0_R |= 1 << (INT_COMP0-16);             // turn-on interrupt 41 (comparator 0)
    COMP_ACINTEN_R|=COMP_ACINTEN_IN0;            //ENABLE INTERRUPS
    waitMicrosecond(1000);
    COMP_ACMIS_R |=1;
    putsUart0("\r\n");

}
if (isCommand("stop",0))
{
    mode=0;
    HIB_IM_R &= ~ HIB_IC_RTCALT0;
    TIMER1_CTL_R &= ~(TIMER_CTL_TAEN);
    NVIC_EN1_R &= ~(1<<(INT_HIBERNATE-16-32));
    COMP_ACINTEN_R &= ~1;
    TIMER1_CTL_R &= ~(TIMER_CTL_TAEN);                  // turn-off timer
    NVIC_EN0_R &= ~(1 << (INT_COMP0-16));             // turn-off interrupt 41 (comparator 0)
    NVIC_EN0_R &= ~(1 << (INT_TIMER1A-16));             // turn-off interrupt 37 (TIMER1A)
    putsUart0("\r\n");
}
if (isCommand("date",3))
{
    month=getValue(0);
    EEpromlocation(4,0);
    EEPROM_EERDWR_R=month;
    date=getValue(1);
    EEpromlocation(5,0);
    EEPROM_EERDWR_R=date;
    year=getValue(2);
    EEpromlocation(6,0);
    EEPROM_EERDWR_R=year;
    putsUart0("\r\n");
}
if (isCommand("date",0))
{


    sprintf(str, " %d  %d  %d\r\n", a,b,c);
                putsUart0(str);



}
if (isCommand("time",3))
{
    starttime=HIB_RTCC_R;
    EEpromlocation(0,0);
    EEPROM_EERDWR_R=starttime;
    hours=getValue(0);
    EEpromlocation(1,0);
    EEPROM_EERDWR_R=hours;
    minutes=getValue(1);
    EEpromlocation(2,0);
    EEPROM_EERDWR_R=minutes;
    seconds=getValue(2);
    EEpromlocation(3,0);
    EEPROM_EERDWR_R=seconds;
    putsUart0("\r\n");
}
if (isCommand("time",0))

{
    elapsedtime=HIB_RTCC_R;
    set_Time();
    sprintf(str, " %d  %d  %d\r\n", d,e,f);
                putsUart0(str);
}

if (isCommand("data",0))
{
    //block 0 offset 0 HIBRTCC at start
    //Block 1 offset 0 hours at start
    //Block 2 offset 0 mins at start
    //Block 3 offset 0 seconds at start
    //block 4 offset 0 months at start
    //block 5          date at start
    //block 6          year at start
    //block 7         sample count
    //block 8          mode
    //block 9          periodic time interval
    //block 10         edge
    //block 11         leveling
    // block 12        temp/voltage


    //char b[11]=["HIBRTCC","hours","Minutes","seconds","months","date","year","samplecount","mode","edge","levelling"];
    uint8_t i;
    for(i=0;i<13;i++)
            {
                EEpromlocation(i,0);
                abc[i]=EEPROM_EERDWR_R;

            }
                sprintf(str, "start date   %d/%d/%d       start Time  %d:%d:%d         \r\n",abc[4],abc[5],abc[6],abc[1],abc[2],abc[3]);
                putsUart0(str);
               // sprintf(str, "HIBRTCC  =  %d \r\n",a[0]);
                //putsUart0(str);
                sprintf(str, "sample count  =  %d \r\n",abc[7]);
                putsUart0(str);


                if(abc[11]==1)
                {
                    putsUart0("leveling is initiated\r\n" );
                }
                else if(abc[11]==0)
                {
                    putsUart0("leveling is not initiated\r\n" );
                }
                else if(abc[8]==1)
                    {
                    {
                        putsUart0("Mode is trigger\r\n");
                        if(abc[10]==1)
                        {
                            putsUart0("edge positive is initiated\r\n ");
                        }
                        else if(abc[10]==0)
                        {
                            putsUart0("edge negetive is initiated\r\n");
                        }
                    }
                        get_firstaddress();
                        for(roll=0;roll<abc[7];roll++)
                        {
                            output= location;

                            get_Nextaddress();
                            get_Nextaddress();
                            get_Nextaddress();
                            get_Nextaddress();

                            hib_value=location;
                            set_Time(hib_value);
                            sprintf(str, "voltage(v)=%3.2f      date  %d/%d/%d  Time %d:%d:%d         \r\n",output/100,a,b,c,d,e,f);
                            putsUart0(str);
                            get_Nextaddress();
                            get_Nextaddress();
                            get_Nextaddress();
                            get_Nextaddress();

                        }
                    }

                else if(abc[8]==2)
                {
                    sprintf(str, "Mode is periodic\r\nperiodic time interval  =  %d seconds\r\n",abc[9]);
                    putsUart0(str);
                    get_firstaddress();
                    hib_value=location;
                    set_Time(hib_value);
                    sprintf(str, "periodic mode initiated at date  %d/%d/%d  Time %d:%d:%d \r\n",a,b,c,d,e,f);
                    putsUart0(str);
                    get_Nextaddress();
                    get_Nextaddress();
                    get_Nextaddress();
                    get_Nextaddress();



                    for(roll=0;roll<abc[7];roll++)
                    {
                        output= location;
                       if(abc[12]==0)
                        {
                            sprintf(str, "voltage(v)=%3.2f\r\n",output/100);
                            putsUart0(str);
                        }
                        else if(abc[12]==1)
                        {
                            sprintf(str, "temperature(C)=%3.2f\r\n",output/100);
                            putsUart0(str);
                        }
                        get_Nextaddress();
                        get_Nextaddress();
                        get_Nextaddress();
                        get_Nextaddress();
                    }
                }





}

}
}
