#include "os_task.h"

extern UART_HandleTypeDef huart4;

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

uint16_t test_pwm=3000;
extern int pg, hg, vg;
os_exec(TEST) {
	(void)argument;
	int8_t tick = 0;
	for (; ; )
	{
		//osDelay(5000);
		GREED_LED_TOGGLE();
		//if(Frame%4 == 0)
			oled_camera_display();
		OLED_PrintN(64, 0, "ok", 0);
		OLED_PrintN(64, 2, "p ", pg);
		OLED_PrintN(64, 4, "h ", hg);
		OLED_PrintN(64, 6, "v ", vg);

			set_pwm_val(TIM_CHANNEL_2,test_pwm);
			test_pwm+=100;
			if (test_pwm == 9000)
				test_pwm = 3000;
		osDelay(50);
	}
}
