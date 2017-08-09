#include "os_task.h"

extern UART_HandleTypeDef huart4;
extern camera_buffer[120][160];
int Frame;
#pragma region ALL_DEFINE

CREATE_TASK_DEFINE(TEST);

#pragma endregion

void os_task_init(void)
{
	CREATE_TASK_HANDLE(TEST, 3);
}

void os_task_start(void)
{
	/* Start scheduler */
	//osThreadSuspend(os_ThreadHandle(TEST));
	osKernelStart();
}


os_exec(TEST) {
	(void)argument;
	int8_t tick = 0;
	for (; ; )
	{
		//osDelay(5000);
		GREED_LED_TOGGLE();
		//if(Frame%4 == 0)
			oled_camera_display();
		osDelay(2);
	}
}
