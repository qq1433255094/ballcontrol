#include "os_task.h"

extern UART_HandleTypeDef huart4;
extern uint8_t Frame_Enable;
extern CAMERA_BUFFER_TYPE camera_buffer[CAMERA_BUFFER_H][CAMERA_BUFFER_W];
extern CAMERA_BUFFER_TYPE camera_frame[CAMERA_BUFFER_H][CAMERA_BUFFER_W];
extern CAMERA_BUFFER_TYPE camera_diff[CAMERA_BUFFER_H][CAMERA_BUFFER_W];

int Frame;
#pragma region ALL_DEFINE

CREATE_TASK_DEFINE(TEST);
CREATE_TASK_DEFINE(CAMERA_CAL);
CREATE_TASK_DEFINE(CONTROL);

#pragma endregion

void os_task_init(void)
{
	CREATE_TASK_HANDLE(TEST, 0);
	CREATE_TASK_HANDLE(CAMERA_CAL, 3);
	CREATE_TASK_HANDLE(CONTROL, 3);
}

void os_task_start(void)
{
	/* Start scheduler */
	//osThreadSuspend(os_ThreadHandle(TEST));
	//osThreadSuspend(os_ThreadHandle(CAMERA_CAL));
	osKernelStart();
}

uint16_t test_pwm=3000;
extern int pg, hg, vg;
uint16_t pix_white;
uint16_t  ball_x = 0, ball_y = 0;

os_exec(TEST) {
	(void)argument;
	int8_t tick = 0;
	int16_t inc = 10;
	for (; ; )
	{
		//osDelay(5000);
		GREED_LED_TOGGLE();
		//if (Frame % 4 == 0)
			oled_camera_display();
		OLED_PrintN(64, 0, "ok", 0);
		OLED_PrintN(64, 2, "x ", ball_x);
		OLED_PrintN(64, 4, "y ", ball_y);
		OLED_PrintN(64, 6, "", pix_white);
	}
}

os_exec(CAMERA_CAL) {
	(void)argument;
	uint16_t cam_h,cam_w,temp,target,w_last,ex_enable;
	for (; ; )
	{
		if (Frame_Enable == ENABLE)
		{
			Frame_Enable = DISABLE;

			temp = 0;
			ex_enable = 0;
			for (cam_h = 0; cam_h < CAMERA_BUFFER_H && ex_enable != 1; cam_h++)
			{
				target = 0;
				for (cam_w = 0; cam_w < CAMERA_BUFFER_W-1 && ex_enable != 1; cam_w++)
				{
					if (camera_frame[cam_h][cam_w] != camera_frame[cam_h][cam_w+1])
					{
						target++;
						
						if (target>1 && cam_w - w_last<3 && camera_frame[cam_h][w_last+1] != 0)
						{
							//osDelay(1);

							for (uint8_t i=0;i<8;i++)
							{
								if ((camera_frame[cam_h][w_last+1] & (0x80>>i)) != 0 )
								{
									ball_x = w_last * 8 + i;
									ball_y = cam_h;
									//osDelay(1);
								}
							}

							ex_enable = 1;

							//ball

						}
						w_last = cam_w;
						//continue;
					}
				}
			}
//			pix_white = temp;
		}
		osDelay(5);
	}
}

os_exec(CONTROL) {
	(void)argument;
	for (; ; )
	{
		pwm_out(TIM_CHANNEL_2, 0);
		pwm_out(TIM_CHANNEL_3, 0);
		osDelay(20);
	}
}
