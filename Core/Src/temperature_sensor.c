#include "temperature_sensor.h"

float temperature_sensor_read(ADC_HandleTypeDef *hadc)
{
    HAL_ADC_Start(hadc);
    HAL_ADC_PollForConversion(hadc, HAL_MAX_DELAY);
    uint32_t adc_value = HAL_ADC_GetValue(hadc);
    HAL_ADC_Stop(hadc);

    float voltaje = (adc_value * 3.3f) / 4095.0f;
    float temperature = voltaje * 100.0f; // LM35DZ: 10mV/°C
    return temperature;
}