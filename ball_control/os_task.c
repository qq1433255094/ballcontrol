#include "os_task.h"
#include "arm_math.h"
#include "cmsis_os.h"

#define MOVE_X 0
#define MOVE_Y 1
#define MOVE_XY 2
#define MOVE_XYR 3

#define ADDR_FLASH_SECTOR_0     ((uint32_t)0x08000000) /* Base @ of Sector 0, 16 Kbytes */
#define ADDR_FLASH_SECTOR_1     ((uint32_t)0x08004000) /* Base @ of Sector 1, 16 Kbytes */
#define ADDR_FLASH_SECTOR_2     ((uint32_t)0x08008000) /* Base @ of Sector 2, 16 Kbytes */
#define ADDR_FLASH_SECTOR_3     ((uint32_t)0x0800C000) /* Base @ of Sector 3, 16 Kbytes */
#define ADDR_FLASH_SECTOR_4     ((uint32_t)0x08010000) /* Base @ of Sector 4, 64 Kbytes */
#define ADDR_FLASH_SECTOR_5     ((uint32_t)0x08020000) /* Base @ of Sector 5, 128 Kbytes */
#define ADDR_FLASH_SECTOR_6     ((uint32_t)0x08040000) /* Base @ of Sector 6, 128 Kbytes */
#define ADDR_FLASH_SECTOR_7     ((uint32_t)0x08060000) /* Base @ of Sector 7, 128 Kbytes */
#define ADDR_FLASH_SECTOR_8     ((uint32_t)0x08080000) /* Base @ of Sector 8, 128 Kbytes */
#define ADDR_FLASH_SECTOR_9     ((uint32_t)0x080A0000) /* Base @ of Sector 9, 128 Kbytes */
#define ADDR_FLASH_SECTOR_10    ((uint32_t)0x080C0000) /* Base @ of Sector 10, 128 Kbytes */
#define ADDR_FLASH_SECTOR_11    ((uint32_t)0x080E0000) /* Base @ of Sector 11, 128 Kbytes */

#define FLASH_USER_START_ADDR   ADDR_FLASH_SECTOR_10

extern UART_HandleTypeDef huart4;
extern uint8_t Frame_Enable;
extern button_HandleTypeDef button1;
extern CAMERA_BUFFER_TYPE camera_buffer[CAMERA_BUFFER_H][CAMERA_BUFFER_W];
extern CAMERA_BUFFER_TYPE camera_frame[CAMERA_BUFFER_H][CAMERA_BUFFER_W];
extern CAMERA_BUFFER_TYPE camera_diff[CAMERA_BUFFER_H][CAMERA_BUFFER_W];
extern uint8_t page_fir[];
extern uint8_t page_sec[];
extern uint8_t area[4];
extern UART_HandleTypeDef huart3;
extern uint8_t receive3[20], transmit3[20];

uint8_t receive_over = 1;

struct Point ball_targer[10] = 
{
	{ 0,0 },
	{ 30,18 }, { 119,15 }, { 207,20 },
	{ 31,107 },{ 119,108 },{ 207,110 },
	{ 30,193 },{ 119,196 },{ 207,193 }
};

struct Point ball_fix[10] =
{
	{ 0,0 },
	{ 5,-5 },{ 5,-5 },{ 10,0 },
	{ 5,5 },{ 5,5 },{ 5,0 },
	{ 5,0 },{ 0,-10 },{ -10,-5 }
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

int16_t test_pwm_2=0;
int16_t test_pwm_3 = 0;
extern int pg, hg, vg;
uint16_t pix_white;
int16_t  ball_x = 0, ball_y = 0;
int16_t  ball_x_last = 0, ball_y_last = 0;
int16_t  ball_vx = 0, ball_vy = 0;
int8_t exec_task = -1;
uint32_t exec_time = 0;
uint32_t time_e;

int16_t  ball_setx = 119, ball_sety = 108;

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
	//reset_para(12, 30);
	ball_sety = ball_targer[2].y;
	ball_setx = ball_targer[2].x;
	osDelay(5000);
}

void base_2() {
	//2
	//reset_para(12, 30);
	ball_sety = ball_targer[1].y;
	ball_setx = ball_targer[1].x;

	move_dis(MOVE_XY, 10, 8, 500);

	ball_sety = ball_targer[5].y+5;
	ball_setx = ball_targer[5].x+5;

	osDelay(5000);
}

void base_3() {
	//reset_para(12, 30);
	ball_sety = ball_targer[1].y;
	ball_setx = ball_targer[1].x;

	move_dis(MOVE_Y, 5, 15, 300);

	ball_sety = ball_targer[4].y+5;
	ball_setx = ball_targer[4].x+5 ;

	osDelay(2000);

	move_dis(MOVE_X, 5, 15, 300);

	ball_sety = ball_targer[5].y;
	ball_setx = ball_targer[5].x+5;

	osDelay(6000);
}

void base_4() {
	//reset_para(12, 35);
	ball_sety = ball_targer[1].y;
	ball_setx = ball_targer[1].x;

	move_dis(MOVE_XY, 10, 6, 400);
	move_dis(MOVE_Y, 10, 8, 400);
	osDelay(1000);
	move_dis(MOVE_X, 10, 10, 400);
	move_dis(MOVE_XY, 5, 6, 300);

	//reset_para(16, 35);
	ball_sety = ball_targer[9].y-5;
	ball_setx = ball_targer[9].x-10;

	osDelay(5000);
}

void pro_1()
{
	reset_para(12, 35);
	ball_sety = ball_targer[1].y;
	ball_setx = ball_targer[1].x;

	move_dis(MOVE_X, 20, 4, 1000);

	ball_sety = ball_targer[2].y-5;
	ball_setx = ball_targer[2].x+5;
	osDelay(1000);

	move_dis(MOVE_XY, 20, 4, 1000);

	ball_sety = ball_targer[6].y;
	ball_setx = ball_targer[6].x+10;
	osDelay(1000);

	move_dis(MOVE_Y, 20, 4, 1000);

	ball_sety = ball_targer[9].y-10;
	ball_setx = ball_targer[9].x-10;
	osDelay(5000);

}

void pro_2()
{
	int16_t A, B, C, D;
	uint32_t time = HAL_GetTick();
	osThreadSuspend(os_ThreadHandle(CONTROL));
	while (area[0]==255 )//&& HAL_GetTick()-time < 120000 )
	{
		osDelay(5);
	}
	if (area[0] != 255)
	{
		time_e = HAL_GetTick();
		A = area[0] - '0';
		B = area[1] - '0';
		C = area[2] - '0';
		D = area[3] - '0';

		receive_over = 1;
		HAL_UART_Receive_IT(&huart3, receive3, 4);

		osThreadResume(os_ThreadHandle(CONTROL));

		//reset_para(8, 30);

		ball_setx = ball_targer[A].x + ball_fix[A].x;
		ball_sety = ball_targer[A].y + ball_fix[A].y;
		osDelay(5000);

		ball_setx = ball_targer[B].x + ball_fix[B].x;
		ball_sety = ball_targer[B].y + ball_fix[B].y;
		osDelay(5000);

		ball_setx = ball_targer[C].x + ball_fix[C].x;
		ball_sety = ball_targer[C].y + ball_fix[C].y;
		osDelay(5000);

		ball_setx = ball_targer[D].x + ball_fix[D].x;
		ball_sety = ball_targer[D].y + ball_fix[D].y;
		osDelay(5000);

		//reset_para(12, 30);
	}
}


void pro_3()
{
	LOOP(3) {
		ball_setx = (ball_targer[5].x + ball_targer[4].x) / 2;
		ball_sety = ball_targer[5].y;
		osDelay(3000);

		ball_setx = ball_targer[5].x;
		ball_sety = (ball_targer[5].y + ball_targer[2].y) / 2;
		osDelay(3000);

		ball_setx = (ball_targer[5].x + ball_targer[6].x) / 2;
		ball_sety = ball_targer[5].y;
		osDelay(3000);

		ball_setx = ball_targer[5].x;
		ball_sety = (ball_targer[5].y + ball_targer[8].y) / 2;
		osDelay(3000);
	}
	ball_setx = (ball_targer[5].x + ball_targer[4].x) / 2;
	ball_sety = ball_targer[5].y;
	osDelay(3000);

	ball_setx = ball_targer[5].x;
	ball_sety = (ball_targer[5].y + ball_targer[2].y) / 2;
	osDelay(3000);

	ball_setx = (ball_targer[5].x + ball_targer[6].x) / 2;
	ball_sety = ball_targer[5].y;
	osDelay(3000);

	ball_setx = ball_targer[9].x - 25;
	ball_sety = ball_targer[9].y - 25;
	osDelay(3000);
	
	ball_setx = ball_targer[9].x -5;
	ball_sety = ball_targer[9].y - 10;
	osDelay(5000);
}


void pro_4()
{

}

FLASH_EraseInitTypeDef EraseInitStruct;
uint32_t SectorError = 0, FlashAddress = 0;
int16_t test1, test2;

static uint32_t GetSector(uint32_t Address)
{
	uint32_t sector = 0;

	if ((Address < ADDR_FLASH_SECTOR_1) && (Address >= ADDR_FLASH_SECTOR_0))
	{
		sector = FLASH_SECTOR_0;
	}
	else if ((Address < ADDR_FLASH_SECTOR_2) && (Address >= ADDR_FLASH_SECTOR_1))
	{
		sector = FLASH_SECTOR_1;
	}
	else if ((Address < ADDR_FLASH_SECTOR_3) && (Address >= ADDR_FLASH_SECTOR_2))
	{
		sector = FLASH_SECTOR_2;
	}
	else if ((Address < ADDR_FLASH_SECTOR_4) && (Address >= ADDR_FLASH_SECTOR_3))
	{
		sector = FLASH_SECTOR_3;
	}
	else if ((Address < ADDR_FLASH_SECTOR_5) && (Address >= ADDR_FLASH_SECTOR_4))
	{
		sector = FLASH_SECTOR_4;
	}
	else if ((Address < ADDR_FLASH_SECTOR_6) && (Address >= ADDR_FLASH_SECTOR_5))
	{
		sector = FLASH_SECTOR_5;
	}
	else if ((Address < ADDR_FLASH_SECTOR_7) && (Address >= ADDR_FLASH_SECTOR_6))
	{
		sector = FLASH_SECTOR_6;
	}
	else if ((Address < ADDR_FLASH_SECTOR_8) && (Address >= ADDR_FLASH_SECTOR_7))
	{
		sector = FLASH_SECTOR_7;
	}
	else if ((Address < ADDR_FLASH_SECTOR_9) && (Address >= ADDR_FLASH_SECTOR_8))
	{
		sector = FLASH_SECTOR_8;
	}
	else if ((Address < ADDR_FLASH_SECTOR_10) && (Address >= ADDR_FLASH_SECTOR_9))
	{
		sector = FLASH_SECTOR_9;
	}
	else if ((Address < ADDR_FLASH_SECTOR_11) && (Address >= ADDR_FLASH_SECTOR_10))
	{
		sector = FLASH_SECTOR_10;
	}
	else /* (Address < FLASH_END_ADDR) && (Address >= ADDR_FLASH_SECTOR_11) */
	{
		sector = FLASH_SECTOR_11;
	}

	return sector;
}


static uint32_t GetSectorSize(uint32_t Sector)
{
	uint32_t sectorsize = 0x00;

	if ((Sector == FLASH_SECTOR_0) || (Sector == FLASH_SECTOR_1) || (Sector == FLASH_SECTOR_2) || (Sector == FLASH_SECTOR_3))
	{
		sectorsize = 16 * 1024;
	}
	else if (Sector == FLASH_SECTOR_4)
	{
		sectorsize = 64 * 1024;
	}
	else
	{
		sectorsize = 128 * 1024;
	}
	return sectorsize;
}

void save_point()
{
	HAL_FLASH_Unlock();

	/* Fill EraseInit structure*/
	EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
	EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;
	EraseInitStruct.Sector = GetSector(FLASH_USER_START_ADDR);
	EraseInitStruct.NbSectors = 1;
	if (HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError) != HAL_OK)
	{
		;
	}

	__HAL_FLASH_DATA_CACHE_DISABLE();
	__HAL_FLASH_INSTRUCTION_CACHE_DISABLE();

	__HAL_FLASH_DATA_CACHE_RESET();
	__HAL_FLASH_INSTRUCTION_CACHE_RESET();

	__HAL_FLASH_INSTRUCTION_CACHE_ENABLE();
	__HAL_FLASH_DATA_CACHE_ENABLE();

	FlashAddress = FLASH_USER_START_ADDR;

	for (int8_t i = 0; i < 10; i++)
	{
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, FlashAddress, ball_targer[i].x);
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, FlashAddress + 2, ball_targer[i].y);
		FlashAddress += 4;
	}
	HAL_FLASH_Lock();

}

void read_point()
{
	FlashAddress = FLASH_USER_START_ADDR;

	for (int8_t i = 0; i < 10; i++)
	{
		ball_targer[i].x = *(__IO int16_t*)(FlashAddress);
		ball_targer[i].y = *(__IO int16_t*)(FlashAddress + 2);
		FlashAddress += 4;
	}
}



os_exec(TEST) {
	(void)argument;
	
	for (; ; )
	{
		time_e = HAL_GetTick();
		switch (exec_task)
		{
		case 0:base_1(); JUMP_FIRST();
			break;
		case 1:base_2(); JUMP_FIRST();
			break;
		case 2:base_3(); JUMP_FIRST();
			break;
		case 3:base_4(); JUMP_FIRST();
			break;
		case 4:pro_1(); JUMP_SECOND();
			break;
		case 5:pro_2(); JUMP_SECOND();
			break;
		case 6:pro_3(); JUMP_SECOND();
			break;
		case 7:pro_4(); JUMP_SECOND();
			break;
		default: exec_task = -1;
			break;
		}

		osThreadSuspend(os_ThreadHandle(CONTROL));
		pwm_out(TIM_CHANNEL_2, 0);
		pwm_out(TIM_CHANNEL_3, 0);
		exec_task = -1;

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
		//exec_time = HAL_GetTick() - time_e;
		oled_camera_display();
		OLED_PrintN(64, 0, "ok", 0);
		OLED_PrintN(64, 2, "x ", ball_x);
		OLED_PrintN(64, 4, "y ", ball_y);
		OLED_PrintN(64, 6, "t ", exec_time/1000);
		osDelay(50);

	}
}
int16_t fix_y=0, fix_x=0;

int8_t fix_enable = 1;
os_exec(KEY) {
	(void)argument;
	pwm_out(TIM_CHANNEL_2, 0);
	pwm_out(TIM_CHANNEL_3, 0);
	for (; ; )
	{
		if (receive3[0] >='0' && receive3[0] <='7'  && receive_over == 1)
		{
			 exec_task = receive3[0] - '0';
			if (exec_task  == 5)
			{
				receive_over = 0;
			}

			receive3[0] = -1;
			osDelay(5);
			area[0] = 255;

			osThreadResume(os_ThreadHandle(TEST));
			if (exec_task != 5)
			{
				osThreadResume(os_ThreadHandle(CONTROL));
			}
			
			HAL_UART_Receive_IT(&huart3, receive3, 4);
		}


		if (button1.state == PLUSE)
		{
			fix_enable = 1;

			if (exec_task != -1)
			{
				osThreadResume(os_ThreadHandle(TEST));
				osThreadResume(os_ThreadHandle(CONTROL));
			}	
		}
		if (button1.state == DOUBLE)
		{
			exec_task++;
			if (exec_task == 8)
			{
				exec_task = -1;
			}
		}
		else if (button1.state == LONG)
		{
			fix_enable = 0;
			osThreadSuspend(os_ThreadHandle(CONTROL));
			JUMP_FIRST();
			pwm_out(TIM_CHANNEL_2, 0);
			pwm_out(TIM_CHANNEL_3, 0);
			
		}
		button1.state = NONE;

		ball_vx = ball_vx*0.2 + 0.8*(ball_x - ball_x_last);
		ball_vy = ball_vy*0.2 + 0.8*(ball_y - ball_y_last);

		ball_x_last = ball_x;
		ball_y_last = ball_y;

		//pwm_out(TIM_CHANNEL_2, test_pwm_2);
		//pwm_out(TIM_CHANNEL_3, test_pwm_3);

		//fix_x = (120 - ball_x)*1;
		//fix_y = (120 - ball_y)*1;

		//if (fix_enable == 1)
		//{
		//	pwm_out(TIM_CHANNEL_2, fix_y + fix_x);
		//	pwm_out(TIM_CHANNEL_3, fix_y - fix_x);
		//}
		

		osDelay(150);
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
			for (cam_h = 2; cam_h < CAMERA_BUFFER_H-15 && ex_enable != 1; cam_h++)
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

							for (uint8_t i=0;i<8;i++)
							{
								if ((camera_frame[cam_h][w_last+1] & (0x80>>i)) != 0 )
								{
									ball_x = w_last * 8 + i;
									ball_y = cam_h;
								}
							}
							ex_enable = 1;
						}
						w_last = cam_w;

					}
				}
			}
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

	for (; ; )
	{	
		int16_t dy = (ball_sety - (int16_t)ball_y);
		int16_t dx = (ball_setx - (int16_t)ball_x);

		if (fabs(dy) > 30)
		{
			pid_y.Kp = 8;
			pid_y.Ki = 0;
			pid_y.Kd = 0;
			arm_pid_init_f32(&pid_y, 1);
			KD = 30;
		}
		else if (fabs(dy) > 8)
		{
			pid_y.Kp = 12;
			pid_y.Ki = 0;
			pid_y.Kd = 0;
			arm_pid_init_f32(&pid_y, 1);
			KD = 30;
		}
		else {
			pid_y.Kp = 18;
			pid_y.Ki = 0;
			pid_y.Kd = 0;
			arm_pid_init_f32(&pid_y, 0);
			KD = 35;
		}

		if (fabs(dx) > 30)
		{
			pid_x.Kp = 6;
			pid_x.Ki = 0;
			pid_x.Kd = 0;
			arm_pid_init_f32(&pid_x, 1);
			KD = 30;
		}
		else if(fabs(dx) > 8)
		{
			pid_x.Kp = 12;
			pid_x.Ki = 0;
			pid_x.Kd = 0;
			arm_pid_init_f32(&pid_x, 1);
			KD = 30;
		}
		else {
			pid_x.Kp = 18;
			pid_x.Ki = 0;
			pid_x.Kd = 0;
			arm_pid_init_f32(&pid_x, 0);
			KD = 35;
		}

		out_y = arm_pid_f32(&pid_y, dy ) - (int16_t)ball_vy * KD;
		out_x = arm_pid_f32(&pid_x, dx ) - (int16_t)ball_vx * KD ;

		pwm_out(TIM_CHANNEL_2, out_y + out_x+ fix_y + fix_x);
		pwm_out(TIM_CHANNEL_3, out_y - out_x+ fix_y - fix_x);		

		osDelay(50);
	}
}
