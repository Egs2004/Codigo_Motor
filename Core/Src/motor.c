#include "motor.h"

#define ENCODER_COUNTS_PER_REVOLUTION 1135

static TIM_HandleTypeDef *pwm_timer;
static TIM_HandleTypeDef *enc_timer;
static int32_t encoder_position;
static uint16_t encoder_position_last_counter;
static uint16_t encoder_speed_last_counter;
static uint32_t encoder_last_time;
static float encoder_rpm;

void Motor_Init(TIM_HandleTypeDef *htim_pwm, TIM_HandleTypeDef *htim_encoder)
{
    pwm_timer = htim_pwm;
    enc_timer = htim_encoder;

    // Arrancar canales PWM
    HAL_TIM_PWM_Start(pwm_timer, TIM_CHANNEL_1); 
    HAL_TIM_PWM_Start(pwm_timer, TIM_CHANNEL_2); 

    // Arrancar el hardware del Encoder
    HAL_TIM_Encoder_Start(enc_timer, TIM_CHANNEL_ALL);
    Motor_ResetEncoder();

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
    return (int32_t)(uint16_t)__HAL_TIM_GET_COUNTER(enc_timer);
}

int32_t Motor_GetEncoderPosition(void)
{
    uint16_t current_counter = (uint16_t)__HAL_TIM_GET_COUNTER(enc_timer);
    int16_t delta = (int16_t)(current_counter - encoder_position_last_counter);

    encoder_position += delta;
    encoder_position_last_counter = current_counter;

    return encoder_position;
}

void Motor_ResetEncoder(void)
{
    __HAL_TIM_SET_COUNTER(enc_timer, 0);
    encoder_position_last_counter = 0;
    encoder_speed_last_counter = 0;
    encoder_position = 0;
    encoder_last_time = HAL_GetTick();
    encoder_rpm = 0.0f;
}

float Motor_GetEncoderRPM(void)
{
    const uint32_t current_time = HAL_GetTick();
    const uint32_t elapsed_ms = current_time - encoder_last_time;
    const uint16_t current_counter =
        (uint16_t)__HAL_TIM_GET_COUNTER(enc_timer);
    const int16_t count_delta =
        (int16_t)(current_counter - encoder_speed_last_counter);

    if (elapsed_ms == 0U)
    {
        return encoder_rpm;
    }

    encoder_rpm = ((float)count_delta * 60000.0f) /
                  ((float)elapsed_ms * ENCODER_COUNTS_PER_REVOLUTION);
    encoder_speed_last_counter = current_counter;
    encoder_last_time = current_time;

    return encoder_rpm;
}