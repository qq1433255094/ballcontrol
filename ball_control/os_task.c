#include "os_task.h"
#include "arm_math.h"

extern UART_HandleTypeDef huart4;
extern uint8_t Frame_Enable;
extern button_HandleTypeDef button1;
extern CAMERA_BUFFER_TYPE camera_buffer[CAMERA_BUFFER_H][CAMERA_BUFFER_W];
extern CAMERA_BUFFER_TYPE camera_frame[CAMERA_BUFFER_H][CAMERA_BUFFER_W];
extern CAMERA_BUFFER_TYPE camera_diff[CAMERA_BUFFER_H][CAMERA_BUFFER_W];
extern uint8_t page_fir[];
extern uint8_t page_sec[];


int Frame;
#pragma region ALL_DEFINE

CREATE_TASK_DEFINE(TEST);
CREATE_TASK_DEFINE(CAMERA_CAL);
CREATE_TASK_DEFINE(CONTROL);
CREATE_TASK_DEFINE(KEY);

#pragma endregion

void os_task_init(void)
{
	CREATE_TASK_HANDLE(TEST, 0);
	CREATE_TASK_HANDLE(KEY, 0);
	CREATE_TASK_HANDLE(CAMERA_CAL, 3);
	CREATE_TASK_HANDLE(CONTROL, 3);
}

void os_task_start(void)
{
	/* Start scheduler */
	//osThreadSuspend(os_ThreadHandle(TEST));
	osThreadSuspend(os_ThreadHandle(CONTROL));
	osKernelStart();
}

uint16_t test_pwm=3000;
extern int pg, hg, vg;
uint16_t pix_white;
int16_t  ball_x = 0, ball_y = 0;
int16_t  ball_x_last = 0, ball_y_last = 0;
int16_t  ball_vx = 0, ball_vy = 0;

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
		OLED_PrintN(64, 6, "k ", button1.state);
		osDelay(50);
	}
}

uint8_t count;

os_exec(KEY) {
	(void)argument;
	for (; ; )
	{
		
		if (button1.state == PLUSE)
		{
			osThreadResume(os_ThreadHandle(CONTROL));
		}
		else if (button1.state == LONG)
		{
			osThreadSuspend(os_ThreadHandle(CONTROL));
			pwm_out(TIM_CHANNEL_2, 0);
			pwm_out(TIM_CHANNEL_3, 0);
		}
		button1.state = NONE;

		ball_vx = ball_x - ball_x_last;
		ball_vy = ball_y - ball_y_last;

		ball_x_last = ball_x;
		ball_y_last = ball_y;

		osDelay(50);
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

int16_t out_y = 0, out_x = 0;
arm_pid_instance_f32 pid_y, pid_x;

float KP = 8;
float KI = 0.;
float KD = -0.4;

os_exec(CONTROL) {
	(void)argument;

	pid_y.Kp = KP;
	pid_y.Ki = KI;
	pid_y.Kd = KD;

	pid_x.Kp = KP;
	pid_x.Ki = KI;
	pid_x.Kd = KD;

	arm_pid_init_f32(&pid_y, 1);
	arm_pid_init_f32(&pid_x, 1);

	//osDelay(500);

	for (; ; )
	{
		//out_y = (120 - (int16_t)ball_y)*8;
		//out_x = (120 - (int16_t)ball_x)*8;
		
		//pid_y.Kp = KP;
		//pid_y.Ki = KI;
		//pid_y.Kd = KD;

		//pid_x.Kp = KP;
		//pid_x.Ki = KI;
		//pid_x.Kd = KD;

		//arm_pid_init_f32(&pid_y, 0);
		//arm_pid_init_f32(&pid_x, 0);

		out_y = arm_pid_f32(&pid_y, (120 - (int16_t)ball_y));
		out_x = arm_pid_f32(&pid_x, (120 - (int16_t)ball_x));
		//out_y = arm_pid_f32(&pid_y, (-(int16_t)ball_vy));
		//out_x = arm_pid_f32(&pid_x, (-(int16_t)ball_vx));

		pwm_out(TIM_CHANNEL_2, out_y + out_x);
		pwm_out(TIM_CHANNEL_3, out_y - out_x);

		

		osDelay(20);
	}
}
