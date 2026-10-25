#include "motor.h"

static TIM_HandleTypeDef *pwm_timer;
static TIM_HandleTypeDef *enc_timer;

void Motor_Init(TIM_HandleTypeDef *htim_pwm, TIM_HandleTypeDef *htim_encoder)
{
    pwm_timer = htim_pwm;
    enc_timer = htim_encoder;

    // Arrancar canales PWM
    HAL_TIM_PWM_Start(pwm_timer, TIM_CHANNEL_1); 
    HAL_TIM_PWM_Start(pwm_timer, TIM_CHANNEL_2); 

    // Arrancar el hardware del Encoder
    HAL_TIM_Encoder_Start(enc_timer, TIM_CHANNEL_ALL);

    // Encender los chips del BTS7960 (R_EN y L_EN unidos a este pin)
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET); 
}

void Motor_SetSpeed(int16_t speed)
{
    // Saturación de seguridad
    if (speed > 1000) speed = 1000;
    if (speed < -1000) speed = -1000;

    if (speed > 0) 
    {
        __HAL_TIM_SET_COMPARE(pwm_timer, TIM_CHANNEL_1, speed);
        __HAL_TIM_SET_COMPARE(pwm_timer, TIM_CHANNEL_2, 0);
    } 
    else if (speed < 0) 
    {
        __HAL_TIM_SET_COMPARE(pwm_timer, TIM_CHANNEL_1, 0);
        __HAL_TIM_SET_COMPARE(pwm_timer, TIM_CHANNEL_2, -speed); // Se invierte el signo
    } 
    else 
    {
        Motor_Stop();
    }
}

void Motor_Stop(void)
{
    __HAL_TIM_SET_COMPARE(pwm_timer, TIM_CHANNEL_1, 0);
    __HAL_TIM_SET_COMPARE(pwm_timer, TIM_CHANNEL_2, 0);
}

int32_t Motor_GetEncoderValue(void)
{
    return __HAL_TIM_GET_COUNTER(enc_timer);
}