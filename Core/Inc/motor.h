#ifndef MOTOR_H
#define MOTOR_H

#include "stm32f1xx_hal.h"

// Inicializa los periféricos del motor
void Motor_Init(TIM_HandleTypeDef *htim_pwm, TIM_HandleTypeDef *htim_encoder);

// Establece la velocidad: -1000 (reversa máxima) a 1000 (adelante máxima)
void Motor_SetSpeed(int16_t speed);

// Detiene el motor (freno)
void Motor_Stop(void);

// Devuelve el conteo crudo del encoder
int32_t Motor_GetEncoderValue(void);

#endif