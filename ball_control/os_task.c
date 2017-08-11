#include "os_task.h"
#include "arm_math.h"
#include "cmsis_os.h"

#define MOVE_X 0
#define MOVE_Y 1
#define MOVE_XY 2
#define MOVE_XYR 3

extern UART_HandleTypeDef huart4;
extern uint8_t Frame_Enable;
extern button_HandleTypeDef button1;
extern CAMERA_BUFFER_TYPE camera_buffer[CAMERA_BUFFER_H][CAMERA_BUFFER_W];
extern CAMERA_BUFFER_TYPE camera_frame[CAMERA_BUFFER_H][CAMERA_BUFFER_W];
extern CAMERA_BUFFER_TYPE camera_diff[CAMERA_BUFFER_H][CAMERA_BUFFER_W];

struct Point ball_targer[10] = 
{
	{ 0,0 },
	{ 30,18 }, { 119,15 }, { 207,20 },
	{ 31,107 },{ 119,108 },{ 207,110 },
	{ 30,193 },{ 119,196 },{ 207,193 }
};
int Frame;

arm_pid_instance_f32 pid_y, pid_x;

float KP = 12;
float KI = 0;
float KD = 30;

#pragma region ALL_DEFINE

CREATE_TASK_DEFINE(TEST);
CREATE_TASK_DEFINE(CAMERA_CAL);
CREATE_TASK_DEFINE(CONTROL);
CREATE_TASK_DEFINE(KEY);
CREATE_TASK_DEFINE(DISPLAY);


#pragma endregion

void os_task_init(void)
{
	CREATE_TASK_HANDLE(TEST, 0);
	CREATE_TASK_HANDLE(DISPLAY, 0);
	CREATE_TASK_HANDLE(KEY, 0);
	CREATE_TASK_HANDLE(CAMERA_CAL, 3);
	CREATE_TASK_HANDLE(CONTROL, 3);
}

void os_task_start(void)
{
	/* Start scheduler */
	osThreadSuspend(os_ThreadHandle(TEST));
	osThreadSuspend(os_ThreadHandle(CONTROL));
	osKernelStart();
}

uint16_t test_pwm=3000;
extern int pg, hg, vg;
uint16_t pix_white;
int16_t  ball_x = 0, ball_y = 0;
int16_t  ball_x_last = 0, ball_y_last = 0;
int16_t  ball_vx = 0, ball_vy = 0;
int16_t  ball_setx = 124, ball_sety = 114;

void move_dis(uint8_t mode, int16_t dis, uint32_t num, uint32_t time)
{
	uint16_t i = 0;
	if (mode == MOVE_X)
	{
		for (i = 0; i < num; i++)
		{
			ball_setx += dis;
			osDelay(time);
		}
	}
	else if (mode == MOVE_Y)
	{
		for (i = 0; i < num; i++)
		{
			ball_sety += dis;
			osDelay(time);
		}
	}
	else if (mode == MOVE_XY)
	{
		for (i = 0; i < num; i++)
		{
			ball_setx += dis;
			ball_sety += dis;
			osDelay(time);
		}
	}
	else if (mode == MOVE_XYR)
	{
		for (i = 0; i < num; i++)
		{
			ball_setx += dis;
			ball_sety -= dis;
			osDelay(time);
		}
	}


}

void reset_para(float p, float d)
{
	KP = p;
	pid_y.Kp = KP;
	pid_x.Kp = KP;

	arm_pid_init_f32(&pid_y, 0);
	arm_pid_init_f32(&pid_x, 0);

	KD = d;
}

void base_1() {
	//1
	reset_para(12, 30);
	ball_sety = ball_targer[2].y;
	ball_setx = ball_targer[2].x;
	osDelay(5000);
}

void base_2() {
	//2
	reset_para(12, 30);
	ball_sety = ball_targer[1].y;
	ball_setx = ball_targer[1].x;

	move_dis(MOVE_XY, 10, 8, 500);

	ball_sety = ball_targer[5].y;
	ball_setx = ball_targer[5].x;

	osDelay(5000);
}

void base_3() {
	reset_para(12, 30);
	ball_sety = ball_targer[1].y;
	ball_setx = ball_targer[1].x;

	move_dis(MOVE_Y, 5, 15, 300);

	ball_sety = ball_targer[4].y;
	ball_setx = ball_targer[4].x + 14;

	osDelay(4000);

	move_dis(MOVE_X, 5, 15, 300);

	ball_sety = ball_targer[5].y;
	ball_setx = ball_targer[5].x;

	osDelay(4000);
}

void base_4() {
	reset_para(12, 35);
	ball_sety = ball_targer[1].y;
	ball_setx = ball_targer[1].x;

	move_dis(MOVE_XY, 10, 6, 500);
	move_dis(MOVE_Y, 10, 8, 500);
	osDelay(1000);
	move_dis(MOVE_X, 10, 8, 500);
	move_dis(MOVE_XY, 10, 4, 500);

	reset_para(16, 35);
	ball_sety = ball_targer[9].y-5;
	ball_setx = ball_targer[9].x-5;

	osDelay(5000);
}

void pro_1()
{
	reset_para(12, 35);
	ball_sety = ball_targer[1].y;
	ball_setx = ball_targer[1].x;

	move_dis(MOVE_X, 5, 15, 400);

	ball_sety = ball_targer[2].y;
	ball_setx = ball_targer[2].x;
	osDelay(1000);

	move_dis(MOVE_XY, 5, 15, 400);

	ball_sety = ball_targer[6].y;
	ball_setx = ball_targer[6].x;
	osDelay(1000);

	move_dis(MOVE_Y, 5, 15, 400);

	ball_sety = ball_targer[9].y;
	ball_setx = ball_targer[9].x;
	osDelay(5000);

}

void pro_2()
{

}


void pro_3()
{

}


void pro_4()
{

}


os_exec(TEST) {
	(void)argument;
	for (; ; )
	{
		
		
		base_3();


		osThreadSuspend(os_ThreadHandle(TEST));
	}
}

os_exec(DISPLAY) {
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
		OLED_PrintN(64, 6, "c ", 0);
		osDelay(50);

	}
}
int16_t fix_y=0, fix_x=0;

int8_t fix_enable = 1;
os_exec(KEY) {
	(void)argument;
	
	for (; ; )
	{
		if (button1.state == PLUSE)
		{
			fix_enable = 1;
			//osThreadResume(os_ThreadHandle(TEST));
			osThreadResume(os_ThreadHandle(CONTROL));
			
		}
		else if (button1.state == LONG)
		{
			fix_enable = 0;
			osThreadSuspend(os_ThreadHandle(CONTROL));
			pwm_out(TIM_CHANNEL_2, 0);
			pwm_out(TIM_CHANNEL_3, 0);
		}
		button1.state = NONE;

		ball_vx = ball_vx*0.5 + 0.5*(ball_x - ball_x_last);
		ball_vy = ball_vy*0.5 + 0.5*(ball_y - ball_y_last);

		ball_x_last = ball_x;
		ball_y_last = ball_y;


		//fix_x = (120 - ball_x)*1;
		//fix_y = (120 - ball_y)*1;

		//if (fix_enable == 1)
		//{
		//	pwm_out(TIM_CHANNEL_2, fix_y + fix_x);
		//	pwm_out(TIM_CHANNEL_3, fix_y - fix_x);
		//}
		

		osDelay(100);
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
						
						if (target>1 && cam_w - w_last<3 && camera_frame[cam_h][w_last+1] != 0 
								&& (camera_frame[cam_h+1][w_last + 1] != 0 
									|| camera_frame[cam_h + 1][w_last + 2] != 0
									|| camera_frame[cam_h + 1][w_last ] != 0))
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


os_exec(CONTROL) {
	(void)argument;

	pid_y.Kp = KP;
	pid_y.Ki = KI;
	pid_y.Kd = 0;

	pid_x.Kp = KP;
	pid_x.Ki = KI;
	pid_x.Kd = 0;

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
		
		int16_t dy = (ball_sety - (int16_t)ball_y);
		int16_t dx = (ball_setx - (int16_t)ball_x);

		//if (dy>20)
		//{
		//	out_y = 500;
		//}
		//else if (dy<-20)
		//{
		//	out_y = -500;
		//}
		//else {
		// 
		//if (fabs(dy) >10)
		//{
		//	pid_y.Kp = 8;
		//	pid_y.Ki = 0;
		//	pid_y.Kd = 0;
		//	arm_pid_init_f32(&pid_y, 1);
		//	KD = 40;
		//}
		//else {
		//	pid_y.Kp = 15;
		//	pid_y.Ki = 0.1;
		//	pid_y.Kd = 0;
		//	arm_pid_init_f32(&pid_y, 0);
		//	KD = 30;
		//}
			out_y = arm_pid_f32(&pid_y, dy ) - (int16_t)ball_vy * KD;
		//}
		
		out_x = arm_pid_f32(&pid_x, dx ) - (int16_t)ball_vx * KD ;
		//out_y = arm_pid_f32(&pid_y, (-(int16_t)ball_vy));
		//out_x = arm_pid_f32(&pid_x, (-(int16_t)ball_vx));

		pwm_out(TIM_CHANNEL_2, out_y + out_x+ fix_y + fix_x);
		pwm_out(TIM_CHANNEL_3, out_y - out_x+ fix_y - fix_x);

		

		osDelay(50);
	}
}
