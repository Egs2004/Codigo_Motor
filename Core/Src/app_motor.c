#include "app_motor.h"
#include "motor.h"
#include "stm32f1xx_hal.h" // Para HAL_GetTick()

typedef enum {
    STATE_FORWARD,
    STATE_PAUSE_1,
    STATE_BACKWARD,
    STATE_PAUSE_2
} MotorState_t;

static MotorState_t current_state = STATE_FORWARD;
static uint32_t last_time = 0;

void App_Motor_Init(void)
{
    current_state = STATE_FORWARD;
    last_time = HAL_GetTick();
}

void App_Motor_Run(void)
{
    uint32_t current_time = HAL_GetTick();

    switch(current_state)
    {
        case STATE_FORWARD:
            Motor_SetSpeed(500); // 50% de velocidad adelante
            if (current_time - last_time >= 3000) // Despues de 3 segundos...
            {
                current_state = STATE_PAUSE_1;
                last_time = current_time;
            }
            break;

        case STATE_PAUSE_1:
            Motor_Stop();
            if (current_time - last_time >= 1000) // Pausa de 1 segundo
            {
                current_state = STATE_BACKWARD;
                last_time = current_time;
            }
            break;

        case STATE_BACKWARD:
            Motor_SetSpeed(-500); // 50% de velocidad en reversa
            if (current_time - last_time >= 3000)
            {
                current_state = STATE_PAUSE_2;
                last_time = current_time;
            }
            break;

        case STATE_PAUSE_2:
            Motor_Stop();
            if (current_time - last_time >= 1000)
            {
                current_state = STATE_FORWARD; // Reinicia el ciclo
                last_time = current_time;
            }
            break;
    }
}