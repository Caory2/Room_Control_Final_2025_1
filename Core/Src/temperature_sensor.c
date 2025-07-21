#include "temperature_sensor.h"
#include "main.h" // Para acceso a hadc1 si se usa ADC
#include "stm32l4xx_hal.h"
#include <math.h>

// Permite acceso al ADC global definido en main.c
extern ADC_HandleTypeDef hadc1;

void temperature_sensor_init(void) {
    // ya esta inicializado en main.c
}

/**
 * @brief Lee la temperatura del sensor conectado al ADC.
 * 
 * Esta función inicia una conversión ADC, espera a que se complete,
 * y luego convierte el valor ADC a grados Celsius.
 * 
 * @return float Temperatura en grados Celsius.
 */
float temperature_sensor_read(void) {
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
    uint32_t adc_value = HAL_ADC_GetValue(&hadc1);
    HAL_ADC_Stop(&hadc1);

    float Vref = 3.3f;
    float R_fixed = 10000.0f; // 10kΩ
    float adc_max = 4095.0f;

    float Vout = (adc_value / adc_max) * Vref;

    float R_ntc = R_fixed * (Vref / Vout - 1);

    float Beta = 3950.0f;
    float T0 = 298.15f;
    float R0 = 10000.0f;

    float tempK = 1.0f / ( (1.0f / T0) + (1.0f / Beta) * log(R_ntc / R0) );
    float tempC = tempK - 273.15f;

    return tempC;
}