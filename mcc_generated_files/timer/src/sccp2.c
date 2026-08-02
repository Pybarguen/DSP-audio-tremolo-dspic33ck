/**
 * SCCP2-TIMER Generated Driver Source File
 * 
 * @file      sccp2.c
 * 
 * @ingroup   timerdriver
 * 
 * @brief     This is the generated driver source file for SCCP2-TIMER driver
 *
 * @skipline @version   Firmware Driver Version 1.6.1
 *
 * @skipline @version   PLIB Version 1.6.6
 *
 * @skipline  Device : dsPIC33CK64MC105
*/

/*
© [2026] Microchip Technology Inc. and its subsidiaries.

    Subject to your compliance with these terms, you may use Microchip 
    software and any derivatives exclusively with Microchip products. 
    You are responsible for complying with 3rd party license terms  
    applicable to your use of 3rd party software (including open source  
    software) that may accompany Microchip software. SOFTWARE IS ?AS IS.? 
    NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS 
    SOFTWARE, INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT,  
    MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT 
    WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY 
    KIND WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF 
    MICROCHIP HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE 
    FORESEEABLE. TO THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP?S 
    TOTAL LIABILITY ON ALL CLAIMS RELATED TO THE SOFTWARE WILL NOT 
    EXCEED AMOUNT OF FEES, IF ANY, YOU PAID DIRECTLY TO MICROCHIP FOR 
    THIS SOFTWARE.
*/

// Section: Included Files

#include <stddef.h> 
#include "../sccp2.h"
#include "../timer_interface.h"

// Section: Data Type Definitions

#define MASK_32_BIT_LOW 0x0000FFFFU
#define MASK_32_BIT_HIGH 0xFFFF0000U

// Section: File specific functions

static void (*SCCP2_TimeoutHandler)(void) = NULL;

// Section: Driver Interface

// Defines an object for TIMER_INTERFACE

const struct TIMER_INTERFACE PotTimer = {
    .Initialize     = &SCCP2_Timer_Initialize,
    .Deinitialize   = &SCCP2_Timer_Deinitialize,
    .Start          = &SCCP2_Timer_Start,
    .Stop           = &SCCP2_Timer_Stop,
    #if TIMER_PERIODCOUNTSET_API_SUPPORT
    .PeriodCountSet = &SCCP2_Timer_PeriodCountSet,
    #endif
    .PeriodSet      = &SCCP2_Timer_PeriodSet,
    .CounterGet     = &SCCP2_Timer_CounterGet,
    .PeriodGet	    = &SCCP2_Timer_PeriodGet,
    .InterruptPrioritySet = &SCCP2_Timer_InterruptPrioritySet,
    .TimeoutCallbackRegister = &SCCP2_Timer_TimeoutCallbackRegister,
    .Tasks          = NULL,
};

// Section: Driver Interface Function Definitions

void SCCP2_Timer_Initialize(void)
{
    // MOD 16-Bit/32-Bit Timer; CCSEL disabled; TMR32 32 Bit; TMRPS 1:1; CLKSEL FOSC/2; TMRSYNC disabled; CCPSLP disabled; CCPSIDL disabled; CCPON disabled; 
    CCP2CON1L = 0x20U; //The module is disabled, till other settings are configured
    //SYNC None; ALTSYNC disabled; ONESHOT disabled; TRIGEN disabled; IOPS Each Time Base Period Match; RTRGEN disabled; OPSRC Timer Interrupt Event; 
    CCP2CON1H = 0x0U;
    //ASDG 0x0; SSDG disabled; ASDGM disabled; PWMRSEN disabled; 
    CCP2CON2L = 0x0U;
    //ICSEL ; AUXOUT Disabled; ICGSM Level-Sensitive mode; OCAEN disabled; OENSYNC disabled; 
    CCP2CON2H = 0x0U;
    //PSSACE Tri-state; POLACE disabled; OSCNT None; OETRIG disabled; 
    CCP2CON3H = 0x0U;
    //ICOV disabled; ICDIS disabled; SCEVT disabled; ASEVT disabled; TRCLR disabled; TRSET disabled; ICGARM disabled; 
    CCP2STATL = 0x0U;
    //TMRL 0x0000; 
    CCP2TMRL = 0x0U;
    //TMRH 0x0000; 
    CCP2TMRH = 0x0U;
    //PRL 38527; 
    CCP2PRL = 0x967FU;
    //PRH 152; 
    CCP2PRH = 0x98U;
    //CMPA 0; 
    CCP2RA = 0x0U;
    //CMPB 0; 
    CCP2RB = 0x0U;
    //BUFL 0x0000; 
    CCP2BUFL = 0x0U;
    //BUFH 0x0000; 
    CCP2BUFH = 0x0U;
    
    SCCP2_Timer_TimeoutCallbackRegister(&SCCP2_TimeoutCallback);

    IFS1bits.CCT2IF = 0;
    // Enabling SCCP2 interrupt
    IEC1bits.CCT2IE = 1;

    CCP2CON1Lbits.CCPON = 1; //Enable Module
}

void SCCP2_Timer_Deinitialize(void)
{
    CCP2CON1Lbits.CCPON = 0;
    
    IFS1bits.CCT2IF = 0;
    IEC1bits.CCT2IE = 0;
    
    CCP2CON1L = 0x0U; 
    CCP2CON1H = 0x0U; 
    CCP2CON2L = 0x0U; 
    CCP2CON2H = 0x100U; 
    CCP2CON3H = 0x0U; 
    CCP2STATL = 0x0U; 
    CCP2TMRL = 0x0U; 
    CCP2TMRH = 0x0U; 
    CCP2PRL = 0xFFFFU; 
    CCP2PRH = 0xFFFFU; 
    CCP2RA = 0x0U; 
    CCP2RB = 0x0U; 
    CCP2BUFL = 0x0U; 
    CCP2BUFH = 0x0U; 
}

void SCCP2_Timer_Start(void)
{
    IFS1bits.CCT2IF = 0;
    // Enable SCCP2 interrupt
    IEC1bits.CCT2IE = 1;
    
    CCP2CON1Lbits.CCPON = 1;
}

void SCCP2_Timer_Stop(void)
{
    CCP2CON1Lbits.CCPON = 0;
    
    IFS1bits.CCT2IF = 0;
    // Disable SCCP2 interrupt
    IEC1bits.CCT2IE = 0;
}

void SCCP2_Timer_PeriodSet(uint32_t count)
{
    if(count > 0xFFFFU)
    {
        CCP2PRL = (uint16_t)(count & MASK_32_BIT_LOW);
        CCP2PRH = (uint16_t)((count & MASK_32_BIT_HIGH) >> 16);
        CCP2CON1Lbits.T32 = 1;
    }
    else
    {
        CCP2PRL = (uint16_t)(count & MASK_32_BIT_LOW);
        CCP2CON1Lbits.T32 = 0;
    }
}

uint32_t SCCP2_Timer_CounterGet(void)
{ 
    uint16_t tmrLow;
    uint16_t tmrHigh;
    uint16_t tmrLowRepeat;
    
    if(CCP2CON1Lbits.T32 == 1)
    {
        tmrLow = CCP2TMRL;
        tmrHigh = CCP2TMRH;
        tmrLowRepeat = CCP2TMRL;
        
        if(tmrLowRepeat < tmrLow)
        {
            return (((uint32_t)CCP2TMRH << 16U) | tmrLowRepeat); 
        }
        else 
        {
            return (((uint32_t)tmrHigh << 16U) | tmrLowRepeat);
        }
    }
    else
    {
        return (uint32_t)CCP2TMRL;
    }
}

void SCCP2_Timer_InterruptPrioritySet(enum INTERRUPT_PRIORITY priority)
{
    IPC6bits.CCT2IP = priority;
}

void SCCP2_Timer_TimeoutCallbackRegister(void (*handler)(void))
{
    if(NULL != handler)
    {
        SCCP2_TimeoutHandler = handler;
    }
}

void SCCP2_TimeoutCallbackRegister(void* handler)
{
    if(NULL != handler)
    {
        SCCP2_TimeoutHandler = handler;
    }
}

void __attribute__ ((weak)) SCCP2_TimeoutCallback (void)
{ 

} 

/* cppcheck-suppress misra-c2012-8.4
*
* (Rule 8.4) REQUIRED: A compatible declaration shall be visible when an object or 
* function with external linkage is defined
*
* Reasoning: Interrupt declaration are provided by compiler and are available
* outside the driver folder
*/
void __attribute__ ( ( interrupt, no_auto_psv ) ) _CCT2Interrupt (void)
{
    if(NULL != SCCP2_TimeoutHandler)
    {
        (*SCCP2_TimeoutHandler)();
    }
    IFS1bits.CCT2IF = 0;
}

#if TIMER_PERIODCOUNTSET_API_SUPPORT
void SCCP2_PeriodCountSet(size_t count)
{
    CCP2PRL = count & MASK_32_BIT_LOW;
    CCP2PRH = (count & MASK_32_BIT_HIGH) >> 16;
}
#endif

/**
 End of File
*/
