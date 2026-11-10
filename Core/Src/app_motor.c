#include "app_motor.h"
#include "motor.h"
#include <stdio.h>

#define EXPERIMENT_SAMPLE_MS 10U
#define PRBS_UPDATE_MS 50U
#define PWM_LOW_PERCENT 35.0f
#define PWM_HIGH_PERCENT 85.0f

static UART_HandleTypeDef *experiment_uart;
static ExperimentMode_t current_experiment;
static uint16_t prbs9_state;
static float current_prbs_pwm;
static float current_pwm;
static uint32_t experiment_time_ms;
static uint32_t last_update_ms;
static uint32_t last_prbs_update_ms;

static void Update_PRBS9(void)
{
    const uint16_t new_bit =
        (uint16_t)(((prbs9_state >> 8) ^ (prbs9_state >> 4)) & 1U);

    prbs9_state = (uint16_t)(((prbs9_state << 1) | new_bit) & 0x01FFU);
    current_prbs_pwm = (new_bit != 0U) ? PWM_HIGH_PERCENT : PWM_LOW_PERCENT;
}

static void SetExperimentPWM(float pwm_percent)
{
    if (pwm_percent < 0.0f)
    {
        pwm_percent = 0.0f;
    }
    else if (pwm_percent > 100.0f)
    {
        pwm_percent = 100.0f;
    }

    current_pwm = pwm_percent;
    Motor_SetSpeed((int16_t)(pwm_percent * 10.0f));
}

static void TransmitSample(void)
{
    char tx_buffer[64];
    const int32_t time_x100 = (int32_t)(experiment_time_ms / 10U);
    const int32_t pwm_x10 = (int32_t)(current_pwm * 10.0f);
    const int32_t rpm_x100 = (int32_t)(Motor_GetEncoderRPM() * 100.0f);
    const int32_t absolute_rpm_x100 =
        (rpm_x100 < 0) ? -rpm_x100 : rpm_x100;
    const int length = snprintf(tx_buffer, sizeof(tx_buffer),
                                "%ld.%02ld,%ld.%01ld,%s%ld.%02ld\r\n",
                                (long)(time_x100 / 100),
                                (long)(time_x100 % 100),
                                (long)(pwm_x10 / 10),
                                (long)(pwm_x10 % 10),
                                (rpm_x100 < 0) ? "-" : "",
                                (long)(absolute_rpm_x100 / 100),
                                (long)(absolute_rpm_x100 % 100));

    if (length > 0 && length < (int)sizeof(tx_buffer))
    {
        HAL_UART_Transmit(experiment_uart, (uint8_t *)tx_buffer,
                          (uint16_t)length, 10U);
    }
}

void App_Motor_Init(UART_HandleTypeDef *huart, ExperimentMode_t mode)
{
    experiment_uart = huart;
    current_experiment = EXP_PRBS9;
    prbs9_state = 0x01FFU;
    current_prbs_pwm = PWM_LOW_PERCENT;
    current_pwm = 0.0f;
    experiment_time_ms = 0U;
    last_update_ms = HAL_GetTick();
    last_prbs_update_ms = 0U;
    Motor_Stop();
}

void App_Motor_Run(void)
{
    const uint32_t now = HAL_GetTick();

    if ((uint32_t)(now - last_update_ms) < EXPERIMENT_SAMPLE_MS)
    {
        return;
    }

    last_update_ms += EXPERIMENT_SAMPLE_MS;

    if (current_experiment == EXP_STEP_73)
    {
        if (experiment_time_ms >= 1000U && experiment_time_ms <= 4000U)
        {
            SetExperimentPWM(73.0f);
        }
        else if (experiment_time_ms > 4000U)
        {
            SetExperimentPWM(0.0f);
            current_experiment = EXP_FINISHED;
        }
        else
        {
            SetExperimentPWM(0.0f);
        }
    }
    else if (current_experiment == EXP_PRBS9)
    {
        if (experiment_time_ms <= 6000U)
        {
            if (experiment_time_ms - last_prbs_update_ms >= PRBS_UPDATE_MS)
            {
                Update_PRBS9();
                last_prbs_update_ms = experiment_time_ms;
            }
            SetExperimentPWM(current_prbs_pwm);
        }
        else
        {
            SetExperimentPWM(0.0f);
            current_experiment = EXP_FINISHED;
        }
    }
    else
    {
        SetExperimentPWM(0.0f);
    }

    if (current_experiment != EXP_IDLE &&
        current_experiment != EXP_FINISHED &&
        experiment_uart != NULL)
    {
        TransmitSample();
    }

    experiment_time_ms += EXPERIMENT_SAMPLE_MS;
}
