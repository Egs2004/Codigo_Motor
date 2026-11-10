#ifndef APP_MOTOR_H
#define APP_MOTOR_H

#include "stm32f1xx_hal.h"

typedef enum
{
    EXP_IDLE = 0,
    EXP_STEP_73 = 1,
    EXP_PRBS9 = 2,
    EXP_FINISHED = 3
} ExperimentMode_t;

/* Cambia a EXP_PRBS9 para ejecutar el experimento PRBS9. */
#define APP_MOTOR_DEFAULT_EXPERIMENT EXP_STEP_73

void App_Motor_Init(UART_HandleTypeDef *huart, ExperimentMode_t mode);
void App_Motor_Run(void);

#endif
