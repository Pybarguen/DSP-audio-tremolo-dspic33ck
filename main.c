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
#include "mcc_generated_files/timer/sccp1.h"
#include "mcc_generated_files/adc/adc1.h"
#include "mcc_generated_files/adc/adc_interface.h"
#define FCY 100000000UL
#include <libpic30.h>


#define UMBRAL_DISTORSION 800
#define UMBRAL_SILENCIO   40
#define Tremolo_Step 13

const uint16_t Tremolo_lut[Tremolo_Step] = {
    102,  // 0.1 (Muy pasito)
    205,  // 0.2
    307,  // 0.3
    512,  // 0.5 (Mitad de volumen)
    716,  // 0.7
    870,  // 0.85
    1024, // 1.0 (Volumen completo original)
    870,  // 0.85 (Empezamos a bajar...)
    716,  // 0.7
    512,  // 0.5
    307,  // 0.3
    205,  // 0.2
    102   // 0.1    
    
};


uint16_t counter = 0;
uint8_t  index = 0;

void get_processing_data()
{
    
    counter++;
    int32_t temp=0;
    int16_t signal_processed=0;
    
    if(counter>=1000){
        
        index++;
        counter = 0;
        
         if(index>=Tremolo_Step)
    {
        
        index=0;
    }
        
        
    }
    int16_t abs_signal;
     
    // Leer directamente el valor del ADC (0 a 4095)
    uint16_t current_data = AdcVoiceInput.ConversionResultGet(Microphone);
    
    
    //Se resta el offset de 1.65V o 2048 del MAX4466 cuando esta en silencio
    int16_t signal_audio = (int16_t)current_data - 2048;
    
    //Se atenua la señal a 0.5 al 50% de su valor
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
    //variable temporal para alojar multiplicacion tremolo
    temp = (int32_t)signal_audio * Tremolo_lut[index];
    
    // Desplazamos para dividir por 1024 y volvemos a guardar en 16 bits
    signal_processed = (int16_t)(temp >> 10);
    
    
    
    //Volver a sumar el offset para el DAC
    int32_t dac_val = (int32_t)signal_processed+ 2048;
    
    // Control de desborde seguro para el hardware
    if (dac_val > 4095) dac_val = 4095;
    if (dac_val < 0)    dac_val = 0;
    
   
    // Escribir directamente al DAC (sin ningún procesamiento)
    CMP1_DACDataWrite(dac_val);
    
   
}
/*
    Main application
*/

int main(void)
{
    SYSTEM_Initialize();
    //LA funcion de llamada del ADC cada vez que se proboca el Trigger del SCCP1
       AdcVoiceInput.CommonCallbackRegister(get_processing_data);
       
       AdcVoiceInput_IndividualChannelInterruptEnable(Microphone);
        __builtin_enable_interrupts(); 
        
        //Inicia el temporizador
        SCCP1_Timer_Start();
    while(1)
    {
    }    
}