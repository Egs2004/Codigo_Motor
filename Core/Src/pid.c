#include "pid.h"

#include <math.h>

#define PID_SAMPLE_TIME_S    0.010f
#define PID_OUTPUT_MIN       (-1000.0f)
#define PID_OUTPUT_MAX       (1000.0f)

static float PID_Clamp(float value, float minimum, float maximum)
{
    if (value < minimum)
    {
        return minimum;
    }

    if (value > maximum)
    {
        return maximum;
    }

    return value;
}

void PID_Init(PID_Controller_t *pid)
{
    if (pid == NULL)
    {
        return;
    }

    pid->Kp = 1.0f;
    pid->Ki = 0.0f;
    pid->Kd = 0.0f;
    PID_Reset(pid);
}

void PID_SetTunings(PID_Controller_t *pid, float kp, float ki, float kd)
{
    if ((pid == NULL) ||
        !isfinite(kp) || !isfinite(ki) || !isfinite(kd) ||
        (kp < 0.0f) || (ki < 0.0f) || (kd < 0.0f))
    {
        return;
    }

    pid->Kp = kp;
    pid->Ki = ki;
    pid->Kd = kd;
}

void PID_Reset(PID_Controller_t *pid)
{
    if (pid == NULL)
    {
        return;
    }

    pid->prev_error = 0.0f;
    pid->integral = 0.0f;
}

int16_t PID_Update(PID_Controller_t *pid, float setpoint, float measurement)
{
    if ((pid == NULL) || !isfinite(setpoint) || !isfinite(measurement))
    {
        return 0;
    }

    const float error = setpoint - measurement;
    const float previous_integral = pid->integral;
    const float derivative = (error - pid->prev_error) / PID_SAMPLE_TIME_S;

    pid->integral += error * PID_SAMPLE_TIME_S;
    pid->integral = PID_Clamp(pid->integral, PID_OUTPUT_MIN, PID_OUTPUT_MAX);

    float output = (pid->Kp * error) +
                   (pid->Ki * pid->integral) +
                   (pid->Kd * derivative);
    output = PID_Clamp(output, PID_OUTPUT_MIN, PID_OUTPUT_MAX);

    // Evita seguir acumulando error cuando la salida ya está saturada.
    if ((output == PID_OUTPUT_MAX && error > 0.0f) ||
        (output == PID_OUTPUT_MIN && error < 0.0f))
    {
        pid->integral = previous_integral;
    }

    pid->prev_error = error;

    return (int16_t)output;
}
