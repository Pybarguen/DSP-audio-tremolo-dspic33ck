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

//Variable para recorrer matriz de procesamiento del tremolo
#define Tremolo_Step 28


/*Array de procesamiento de señal del tremolo
 * este array multiplica el valor original de la señal por
 * cada paso utilizando la ariemtica de punto fijo formato Q10
 * /
 */

const uint16_t Tremolo_lut[Tremolo_Step] = {
    1024, 1009, 966, 898, 809, 702, 584, 
     461, 343, 235, 142,  68,  20,   1, 
       1,  20,  68, 142, 235, 343, 461, 
     584, 702, 809, 898, 966,1009,1024
};


//Variables para tomar los datos actuales del potenciometro y del microfono ADC
uint16_t current_data_pot = 0;
uint16_t current_data_mic=0;

//Variable temportal para procesar señal original con el array del tremolo
int32_t temp=0;
int32_t dac_val=0;

//Variables procesamiento tremolo
uint16_t counter = 0;
uint8_t  index = 0;
uint8_t tremole_state=0;

//Estructura para funcion antirebote del BOTON Activacion tremolo
typedef struct {
    uint8_t contador;
    uint8_t estado_previo;
} DebounceBtn;

// Creamos una instancia para cada bot n
DebounceBtn Tremolo_on_off = {0, 1};

//Funcion para evaluar antirebote del boton
uint8_t botonPresionado(uint8_t nivel_pin, DebounceBtn *btn) {
    if (nivel_pin == 1) { 
        if (btn->contador < 5) {
            btn->contador++;
        } else if (btn->estado_previo == 1) {
            btn->estado_previo = 0;
            return 1; //  Pulsaci n validada!
        }
    } else {
        btn->contador = 0;
        btn->estado_previo = 1;
    }
    return 0; // No hay pulsaci n nueva
}


/*Funcion para procesar el canal ADC del microfono conectado a RB0
 * La funcion obtiene la señal y evalua si el efecto tremolo esta activo
 * si el efecto tremolo esta activo procesa la señal y activa el CANAL DAC
 * con la señal del tremolo procesada
 * 
 * Si el efecto tremolo no esta activo, envia la señal original al canal DAC
 */
void Get_mic()
{
    
    counter++;
    int32_t temp=0;
    int16_t signal_processed=0;
    
     // 1. Disparar conversión por software dentro de la interrupción del timer
    ADCAPP.SoftwareTriggerEnable();
  
    // Leemos el pin usando la macro del MCC
    
    
    
    
  
    
        
    
     
     
    
    
    int16_t abs_signal;
     
    // Leer directamente el valor del ADC (0 a 4095)
    uint16_t current_data =  ADCAPP.ConversionResultGet(Microphone);
    
    
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
    
    
    if(tremole_state==1)
    {
        
        uint32_t tremole_speed = (uint32_t)(1000UL * (uint32_t)current_data_pot) / 4098UL;
     
    
     if (tremole_speed < 5) 
{
    tremole_speed = 5; 
}
     
     
    
    if(counter>=tremole_speed ){
        
        index++;
        counter = 0;
        
         if(index>=Tremolo_Step)
    {
        
        index=0;
    }
        
        
    }
    //variable temporal para alojar multiplicacion tremolo
    temp = (int32_t)signal_audio * Tremolo_lut[index];
    
    // Desplazamos para dividir por 1024 y volvemos a guardar en 16 bits
    signal_processed = (int16_t)(temp >> 10);
    
     //Volver a sumar el offset para el DAC
   dac_val = (int32_t)signal_processed+ 2048;
    
    }
    else
    {
       dac_val = (int32_t)signal_audio+ 2048;
        
    }
    
    
    
   
    
    // Control de desborde seguro para el hardware
    if (dac_val > 4095) dac_val = 4095;
    if (dac_val < 0)    dac_val = 0;
    
   
    // Escribir directamente al DAC (sin ningún procesamiento)
    CMP1_DACDataWrite(dac_val);
    
   
}
       


/* Esta función se ejecutará automáticamente cada 100 ms gracias al SCCP2 (PotTimer)
 * Esta funcion obtiene el valor del canal ADC conectado con el potenciometro canal de 12bits
 * de acuerdo a su valor se procesa la señal original con el ARRAY del efecto tremolo
 * */

void Get_pot(void)
{
     
     // 1. Disparar conversión por software dentro de la interrupción del timer
    ADCAPP.SoftwareTriggerEnable();
    
    // 2. Esperar a que termine la conversión del canal del potenciómetro
    while(!ADCAPP.IsConversionComplete(Tremole_speed)); // Ajusta Channel_AN5 al canal real de tu pote
    
    // 3. Leer el resultado
   current_data_pot = ADCAPP.ConversionResultGet(Tremole_speed); 
    
    
   uint8_t estado_pin = Button_GetValue();
   
   //Condicion para evaluar si se ha presionado el boton utilizando la funcion actirebote
   if (botonPresionado(estado_pin, &Tremolo_on_off)) {
        
     
       tremole_state = !tremole_state;
    }
   
   
       
    if(tremole_state==1)//Si el efecto tremolo esta activo : 
    {
        
        Led_SetLow();//Se enciende el LED
    }
    
   else if(tremole_state==0)//si no
    {
        
        Led_SetHigh();//Se apaga el led
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