#ifndef PID_H
#define PID_H

#include <stdint.h>

// Controlador PID discreto. La salida está limitada al rango de Motor_SetSpeed.
typedef struct {
    float Kp;
    float Ki;
    float Kd;
    
    // Variables de estado internas
    float prev_error;
    float integral;
} PID_Controller_t;

// Inicializa el controlador con ganancias proporcionales por defecto.
void PID_Init(PID_Controller_t *pid);

// Configura las ganancias del controlador. Los valores deben ser finitos y no negativos.
void PID_SetTunings(PID_Controller_t *pid, float kp, float ki, float kd);

// Reinicia el estado sin modificar las ganancias configuradas.
void PID_Reset(PID_Controller_t *pid);

// Ejecuta un ciclo de control de 10 ms y devuelve una salida entre -1000 y 1000.
int16_t PID_Update(PID_Controller_t *pid, float setpoint, float measurement);

#endif /* PID_H */
