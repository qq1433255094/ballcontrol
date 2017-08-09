#pragma once
#pragma once
#ifndef __OS_TASK_H__
#define __OS_TASK_H__

#define GREED_LED_ON()		HAL_GPIO_WritePin (GPIOE, GPIO_PIN_3, GPIO_PIN_RESET)
#define GREED_LED_OFF()		HAL_GPIO_WritePin (GPIOE, GPIO_PIN_3, GPIO_PIN_SET)
#define GREED_LED_TOGGLE()	HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_3)

#define BLUE_LED_ON()			HAL_GPIO_WritePin (GPIOE, GPIO_PIN_2, GPIO_PIN_RESET)
#define BLUE_LED_OFF()			HAL_GPIO_WritePin (GPIOE, GPIO_PIN_2, GPIO_PIN_SET)
#define BLUE_LED_TOGGLE()		HAL_GPIO_TogglePin(GPI0E, GPIO_PIN_2)


#define CREATE_TASK_DEFINE(name)		uint8_t name;osThreadId name##ThreadHandle; static void name##_Thread(void const * argument)
#define CREATE_TASK_HANDLE(name,level)	osThreadDef(name##Handle, name##_Thread, osPriorityNormal, level, configMINIMAL_STACK_SIZE);\
										name##ThreadHandle = osThreadCreate(osThread(name##Handle), NULL)
#define os_ThreadHandle(name)			name##ThreadHandle
#define os_exec(name)					static void name##_Thread(void const * argument)

#ifdef __cplusplus
extern "C" {
#endif

#include <stm32f4xx_hal.h>
#include <../CMSIS_RTOS/cmsis_os.h>
#include "ov7670.h"
#include "uart4.h"
#include "usart6.h"
#include "usart3.h"
#include "tim3.h"
#include "tim4.h"
#include "button.h"
#include "can2.h"
#include "rng.h"
#include "mymath.h"
#include <math.h>
#include "arm_math.h"
#include "oled.h"
#include "pwm.h"

	void os_task_init(void);
	void os_task_start(void);


#ifdef __cplusplus
}
#endif

#endif
