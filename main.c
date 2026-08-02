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
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "mcc_generated_files/system/pins.h"
#include "mcc_generated_files/system/system.h"
#include "mcc_generated_files/adc/adc1.h"
#include "mcc_generated_files/adc/adc_interface.h"
#include "mcc_generated_files/timer/sccp2.h"
#include "mcc_generated_files/timer/sccp1.h"
#define FCY 100000000UL
#include <libpic30.h>

extern const struct ADC_INTERFACE ADCAPP;

uint16_t current_data_pot = 0;
uint16_t current_data_mic=0;
void Get_mic(void)
{
       int16_t abs_signal;
       
       
     // 1. Disparar conversión por software dentro de la interrupción del timer
    ADCAPP.SoftwareTriggerEnable();
    
    // 2. Esperar a que termine la conversión del canal del potenciómetro
    while(!ADCAPP.IsConversionComplete(Microphone)); // Ajusta Channel_AN5 al canal real de tu pote
    
    // 3. Leer el resultado
    current_data_mic = ADCAPP.ConversionResultGet(Microphone); 
    
    //Se resta el offset de 1.65V o 2048 del MAX4466 cuando esta en silencio
    int16_t signal_audio = (int16_t)current_data_mic - 2048;
    
//        Se atenua la señal a 0.5 al 50% de su valor
    signal_audio = (signal_audio>>1);
    
     //Variable para guardar el valor absoluto de la señal.
    abs_signal = signal_audio;
    
    //Si el valor absoluto es menor que 0
    if(abs_signal<0)
    {
        //Se convierte el valor a numero positivo
       abs_signal = abs_signal *(-1);
        
        
    }
    //Se evalua si la señal es ruido, si es asi se modifica la señal original a 0
    if(abs_signal<25){
        
        signal_audio = 0;
        
    }
    
     //Volver a sumar el offset para el DAC
    int32_t dac_val = (int32_t)signal_audio+ 2048;
    
    // Escribir directamente al DAC (sin ningún procesamiento)
    CMP1_DACDataWrite(dac_val);
    
  
    
}

// Esta función se ejecutará automáticamente cada 100 ms gracias al SCCP2 (PotTimer)
void Get_pot(void)
{
     
     // 1. Disparar conversión por software dentro de la interrupción del timer
    ADCAPP.SoftwareTriggerEnable();
    
    // 2. Esperar a que termine la conversión del canal del potenciómetro
    while(!ADCAPP.IsConversionComplete(Tremole_speed)); // Ajusta Channel_AN5 al canal real de tu pote
    
    // 3. Leer el resultado
   current_data_pot = ADCAPP.ConversionResultGet(Tremole_speed); 
    
    // 4. Controlar el LED
    if(current_data_pot >= 2048)
    {
        Led_SetLow();
    }
    else
    {
        Led_SetHigh();
    }
}

int main(void)
{
    // Inicializa todo el hardware configurado en el MCC (incluyendo pines, ADC y el SCCP2)
    SYSTEM_Initialize();
    
    __builtin_enable_interrupts();
    
   // Registrar la función para que el SCCP1 (MicTimer) la llame en su interrupción
    Mic_Timer.TimeoutCallbackRegister(Get_mic);
    
    Mic_Timer.Start();
    
    //Registrar la función para que el SCCP2 (PotTimer) la llame en su interrupción
   PotTimer.TimeoutCallbackRegister(Get_pot); // Nota: Dependiendo de la versión del MCC, puede ser PotTimer.TimeoutCallbackRegister o PotTimer_TimeoutCallbackRegister
    
    // Arrancar el temporizador del potenciómetro
   PotTimer.Start();
   
    while(1)
    {
        // El bucle principal se queda libre para cuando implementes el procesamiento de audio pesado
    }    
}