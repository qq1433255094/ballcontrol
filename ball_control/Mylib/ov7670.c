#include "ov7670.h"

#define CAMERA_WRITE_ADDR	0x42
#define CAMERA_READ_ADDR	0x42
#define OV7670_REG_NUM  		168
#define OV7725_REG_NUM  		49

#define I2C_TIMEOUT  100

#define CAMERA_I2C			I2C1
#define CAMERA_I2C_SPEED    100000
#define CAMERA_I2C_CLK_ENABLE()               __I2C1_CLK_ENABLE()
#define CAMERA_DMA_CLK_ENABLE()               __DMA1_CLK_ENABLE()
#define CAMERA_I2C_SCL_SDA_GPIO_CLK_ENABLE()  __GPIOB_CLK_ENABLE()

#define CAMERA_I2C_FORCE_RESET()              __I2C1_FORCE_RESET()
#define CAMERA_I2C_RELEASE_RESET()            __I2C1_RELEASE_RESET()

/* Definition for I2Cx Pins */
#define CAMERA_I2C_SCL_PIN                    GPIO_PIN_6
#define CAMERA_I2C_SCL_SDA_GPIO_PORT          GPIOB
#define CAMERA_I2C_SCL_SDA_AF                 GPIO_AF4_I2C1
#define CAMERA_I2C_SDA_PIN                    GPIO_PIN_9

/* I2C interrupt requests */
#define CAMERA_I2C_EV_IRQn                    I2C1_EV_IRQn
#define CAMERA_I2C_ER_IRQn                    I2C1_ER_IRQn

#define SCCB_SIC_H()     HAL_GPIO_WritePin (GPIOB, GPIO_PIN_4, GPIO_PIN_SET)
#define SCCB_SIC_L()    HAL_GPIO_WritePin (GPIOB, GPIO_PIN_4, GPIO_PIN_RESET)
#define SCCB_SID_H()     HAL_GPIO_WritePin (GPIOB, GPIO_PIN_5, GPIO_PIN_SET)
#define SCCB_SID_L()     HAL_GPIO_WritePin (GPIOB, GPIO_PIN_5, GPIO_PIN_RESET)
#define SCCB_DATA_IN      SCCB_SID_IN()
#define SCCB_DATA_OUT     SCCB_SID_OUT()
#define SCCB_SID_STATE	 HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_5)


I2C_HandleTypeDef  camera_I2c;
static DCMI_HandleTypeDef hdcmi_camera;
uint8_t test_val = 0;
uint32_t camera_buffer[120][160];

void CAMERA_I2C_Init();
void CAMERA_I2C_MspInit();
uint8_t CAMERA_I2C_Read(uint8_t Reg);
void CAMERA_I2C_Write(uint8_t Reg, uint8_t Value);

static void CAMERA_I2C_Error(uint8_t Addr);

void SCCB_Init(void);
void SCCB_SID_OUT(void);
void SCCB_SID_IN(void);
void SCCB_Start(void);
void SCCB_Stop(void);
void noAck(void);
uint8_t SCCB_Write(uint8_t m_data);
uint8_t SCCB_Read(void);
uint8_t OV_WriteReg(uint8_t regID, uint8_t regDat);
uint8_t OV_ReadReg(uint8_t regID, uint8_t *regDat);
void OV_Reset(void);
uint8_t OV_ReadID(void);

static void DCMI_MspInit(void);
void Cam_Init();

static const uint8_t OV7670_reg[OV7670_REG_NUM][2] =
{
	{ 0x00, 0x00 },//AGC 
	{ 0x01, 0x40 },
	{ 0x02, 0x40 },
	{ 0x03, 0x0a },//0x0a, 		
	{ 0x09, 0x02 },
	{ 0x0c, 0x00 },
	{ 0x0d, 0x00 },
	{ 0x0e, 0x61 },
	{ 0x0f, 0x43 },
	{ 0x10, 0x00 },
	{ 0x11, 0x80 },
	{ 0x12, 0x11 }, //COM7 
	{ 0x13, 0xe0 },
	{ 0x13, 0xe5 },
	{ 0x13, 0xe3 },//e7
	{ 0x14, 0x18 },//0x38, limit the max gain 
	{ 0x15, 0x08 }, //重要参数 
	{ 0x16, 0x02 },
	{ 0x17, 0x16 },
	{ 0x18, 0x04 },
	{ 0x19, 0x02 },
	{ 0x1a, 0x7a },//0x7a, 
	{ 0x1e, 0x07 },//0x07, 
	{ 0x21, 0x02 },
	{ 0x22, 0x91 },
	{ 0x24, 0x75 },
	{ 0x25, 0x63 },
	{ 0x26, 0xA5 },
	{ 0x29, 0x07 },
	{ 0x32, 0x80 },
	{ 0x33, 0x0b },
	{ 0x34, 0x11 },
	{ 0x35, 0x0b },
	{ 0x37, 0x1d },
	{ 0x38, 0x71 },
	{ 0x39, 0x2a },
	{ 0x3a, 0x14 },
	{ 0x3b, 0x02 },//0x00,//0x02, 
	{ 0x3b, 0x42 },//0x82,//0xc0,//0xc2,	//night mode 
	{ 0x3c, 0x78 },
	{ 0x3d, 0x80 },//0xc0, 
	{ 0x3e, 0x00 },// 
	{ 0x3f, 0x05 },//边缘增强调整 
	{ 0x40, 0xc0 },
	{ 0x41, 0x08 },
	{ 0x41, 0x38 },
	{ 0x43, 0x14 },
	{ 0x44, 0xf0 },
	{ 0x45, 0x34 },
	{ 0x46, 0x58 },
	{ 0x47, 0x28 },
	{ 0x48, 0x3a },
	{ 0x4b, 0x09 },
	{ 0x4c, 0x0F },//噪声抑制强度 
	{ 0x4d, 0x40 },
	{ 0x4e, 0x20 },
	{ 0x4f, 0x80 },
	{ 0x50, 0x80 },
	{ 0x51, 0x00 },
	{ 0x52, 0x22 },
	{ 0x53, 0x5e },
	{ 0x54, 0x80 },
	{ 0x55, 0x0A },//亮度 
	{ 0x56, 0x4f },//对比度 
	{ 0x58, 0x9e },
	{ 0x59, 0x88 },
	{ 0x5a, 0x88 },
	{ 0x5b, 0x44 },
	{ 0x5c, 0x67 },
	{ 0x5d, 0x49 },
	{ 0x5e, 0x0e },
	{ 0x64, 0x04 },
	{ 0x65, 0x20 },
	{ 0x66, 0x05 },
	{ 0x67, 0x11 },
	{ 0x68, 0xFF },
	{ 0x69, 0x55 },
	{ 0x6a, 0x40 },
	{ 0x6b, 0x0A },//PLL 重要参数 
	{ 0x6c, 0x0a },
	{ 0x6d, 0x55 },
	{ 0x6e, 0x11 },
	{ 0x6f, 0x9f },//0x9e for advance AWB 
	{ 0x70, 0x00 },
	{ 0x71, 0x01 },
	{ 0x72, 0x11 },
	{ 0x73, 0x00 },// 
	{ 0x74, 0x19 },
	{ 0x75, 0x05 },
	{ 0x76, 0xe1 },
	{ 0x77, 0x0a },
	{ 0x78, 0x04 },
	{ 0x79, 0x01 },
	{ 0x79, 0x02 },
	{ 0x79, 0x03 },
	{ 0x79, 0x05 },
	{ 0x79, 0x09 },
	{ 0x79, 0x0a },
	{ 0x79, 0x0b },
	{ 0x79, 0x0c },
	{ 0x79, 0x0d },
	{ 0x79, 0x0f },
	{ 0x79, 0x10 },
	{ 0x79, 0x26 },
	{ 0x7a, 0x20 },
	{ 0x7b, 0x1c },
	{ 0x7c, 0x28 },
	{ 0x7d, 0x3c },
	{ 0x7e, 0x55 },
	{ 0x7f, 0x68 },
	{ 0x80, 0x76 },
	{ 0x81, 0x80 },
	{ 0x82, 0x88 },
	{ 0x83, 0x8f },
	{ 0x84, 0x96 },
	{ 0x85, 0xa3 },
	{ 0x86, 0xaf },
	{ 0x87, 0xc4 },
	{ 0x88, 0xd7 },
	{ 0x89, 0xe8 },
	{ 0x8d, 0x4f },
	{ 0x8e, 0x00 },
	{ 0x8f, 0x00 },
	{ 0x90, 0x00 },
	{ 0x91, 0x00 },
	{ 0x92, 0x00 },//0x19,//0x66 
	{ 0x94, 0x04 },
	{ 0x95, 0x08 },
	{ 0x96, 0x00 },
	{ 0x96, 0x00 },
	{ 0x97, 0x30 },
	{ 0x98, 0x20 },
	{ 0x99, 0x30 },
	{ 0x9a, 0x80 },
	{ 0x9a, 0x84 },
	{ 0x9b, 0x29 },
	{ 0x9c, 0x03 },
	{ 0x9d, 0x4c },
	{ 0x9e, 0x3f },
	{ 0x9f, 0x78 },
	{ 0xa0, 0x68 },
	{ 0xa1, 0x03 },//0x0b, 
	{ 0xa2, 0x02 },
	{ 0xa4, 0x89 },//0x88, 
	{ 0xa5, 0x05 },
	{ 0xa6, 0xdf },//0xd8, 
	{ 0xa7, 0xdf },//0xd8, 
	{ 0xa8, 0xf0 },
	{ 0xa9, 0x90 },
	{ 0xaa, 0x94 },
	{ 0xab, 0x07 },
	{ 0xb0, 0x84 },
	{ 0xb1, 0x0c },
	{ 0xb2, 0x0e },
	{ 0xb3, 0x82 },
	{ 0xb8, 0x0a },
	{ 0xc8, 0x00 },
	{ 0xc8, 0x01 },
	{ 0xc8, 0x0f },
	{ 0xc8, 0x20 },
	{ 0xc8, 0x30 },
	{ 0xc8, 0x40 },
	{ 0xc8, 0x7e },
	{ 0xc8, 0x80 },
	{ 0xc8, 0x80 },
	{ 0xc8, 0xc0 },
	{ 0xc8, 0xf0 },
	{ 0xc9, 0x60 }
};


/*OV7725初始化配置表*/
static const uint8_t ov7725_eagle_reg_one[OV7725_REG_NUM][2] =
{
	/*
	COM4	CLKRC
	0xC1	0x02	==> 50帧
	0x41	0x00	==> 75帧
	0x81	0x00	==> 112帧
	0xC1	0x00	==> 150帧
	*/
	//寄存器，寄存器值次
	{ OV7725_COM4         , 0xC1 },		//150帧
	{ OV7725_CLKRC        , 0x00 },
	{ OV7725_COM2         , 0x03 },
	{ OV7725_COM3         , 0xD0 },
	{ OV7725_COM7         , 0x40 },
	{ OV7725_HSTART       , 0x3F },
	{ OV7725_HSIZE        , 0x50 },
	{ OV7725_VSTRT        , 0x03 },
	{ OV7725_VSIZE        , 0x78 },
	{ OV7725_HREF         , 0x00 },
	{ OV7725_SCAL0        , 0x0A },
	{ OV7725_AWB_Ctrl0    , 0xE0 },
	{ OV7725_DSPAuto      , 0xff },
	{ OV7725_DSP_Ctrl2    , 0x0C },
	{ OV7725_DSP_Ctrl3    , 0x00 },
	{ OV7725_DSP_Ctrl4    , 0x00 },
#if (CAMERA_W == 80)
	{ OV7725_HOutSize     , 0x14 },
#elif (CAMERA_W == 160)
	{ OV7725_HOutSize     , 0x28 },
#elif (CAMERA_W == 240)
	{ OV7725_HOutSize     , 0x3c },
#elif (CAMERA_W == 320)
	{ OV7725_HOutSize     , 0x50 },
#else
#endif
#if (CAMERA_H == 60 )
	{ OV7725_VOutSize     , 0x1E },
#elif (CAMERA_H == 120 )
	{ OV7725_VOutSize     , 0x3c },
#elif (CAMERA_H == 180 )
	{ OV7725_VOutSize     , 0x5a },
#elif (CAMERA_H == 240 )
	{ OV7725_VOutSize     , 0x78 },
#else
#endif
	{ OV7725_EXHCH        , 0x00 },
	{ OV7725_GAM1         , 0x0c },
	{ OV7725_GAM2         , 0x16 },
	{ OV7725_GAM3         , 0x2a },
	{ OV7725_GAM4         , 0x4e },
	{ OV7725_GAM5         , 0x61 },
	{ OV7725_GAM6         , 0x6f },
	{ OV7725_GAM7         , 0x7b },
	{ OV7725_GAM8         , 0x86 },
	{ OV7725_GAM9         , 0x8e },
	{ OV7725_GAM10        , 0x97 },
	{ OV7725_GAM11        , 0xa4 },
	{ OV7725_GAM12        , 0xaf },
	{ OV7725_GAM13        , 0xc5 },
	{ OV7725_GAM14        , 0xd7 },
	{ OV7725_GAM15        , 0xe8 },
	{ OV7725_SLOP         , 0x20 },
	{ OV7725_LC_RADI      , 0x00 },
	{ OV7725_LC_COEF      , 0x13 },
	{ OV7725_LC_XC        , 0x08 },
	{ OV7725_LC_COEFB     , 0x14 },
	{ OV7725_LC_COEFR     , 0x17 },
	{ OV7725_LC_CTR       , 0x05 },
	{ OV7725_BDBase       , 0x99 },
	{ OV7725_BDMStep      , 0x03 },
	{ OV7725_SDE          , 0x04 },
	{ OV7725_BRIGHT       , 0x00 },
	{ OV7725_CNST         , TWOchNum },	//二值化阈值设置；57
	{ OV7725_SIGN         , 0x06 },
	{ OV7725_UVADJ0       , 0x11 },
	{ OV7725_UVADJ1       , 0x02 },
};


/*OV7725初始化配置表*/
static const uint8_t ov7725_eagle_reg_two[OV7725_REG_NUM][2] =
{
	/*
	COM4	CLKRC
	0xc1	0x02	==> 50帧
	0x41	0x00	==> 75帧
	0x81	0x00	==> 112帧
	0xc1	0x00	==> 150帧
	*/
	//寄存器，寄存器值次
	{ OV7725_COM4         , 0x81 },		//112帧
	{ OV7725_CLKRC        , 0x00 },
	{ OV7725_COM2         , 0x03 },
	{ OV7725_COM3         , 0xD0 },
	{ OV7725_COM7         , 0x40 },
	{ OV7725_HSTART       , 0x3F },
	{ OV7725_HSIZE        , 0x50 },
	{ OV7725_VSTRT        , 0x03 },
	{ OV7725_VSIZE        , 0x78 },
	{ OV7725_HREF         , 0x00 },
	{ OV7725_SCAL0        , 0x0A },
	{ OV7725_AWB_Ctrl0    , 0xE0 },
	{ OV7725_DSPAuto      , 0xff },
	{ OV7725_DSP_Ctrl2    , 0x0C },
	{ OV7725_DSP_Ctrl3    , 0x00 },
	{ OV7725_DSP_Ctrl4    , 0x00 },
#if (CAMERA_W == 80)
	{ OV7725_HOutSize     , 0x14 },
#elif (CAMERA_W == 160)
	{ OV7725_HOutSize     , 0x28 },
#elif (CAMERA_W == 240)
	{ OV7725_HOutSize     , 0x3c },
#elif (CAMERA_W == 320)
	{ OV7725_HOutSize     , 0x50 },
#else
#endif
#if (CAMERA_H == 60 )
	{ OV7725_VOutSize     , 0x1E },
#elif (CAMERA_H == 120 )
	{ OV7725_VOutSize     , 0x3c },
#elif (CAMERA_H == 180 )
	{ OV7725_VOutSize     , 0x5a },
#elif (CAMERA_H == 240 )
	{ OV7725_VOutSize     , 0x78 },
#else
#endif
	{ OV7725_EXHCH        , 0x00 },
	{ OV7725_GAM1         , 0x0c },
	{ OV7725_GAM2         , 0x16 },
	{ OV7725_GAM3         , 0x2a },
	{ OV7725_GAM4         , 0x4e },
	{ OV7725_GAM5         , 0x61 },
	{ OV7725_GAM6         , 0x6f },
	{ OV7725_GAM7         , 0x7b },
	{ OV7725_GAM8         , 0x86 },
	{ OV7725_GAM9         , 0x8e },
	{ OV7725_GAM10        , 0x97 },
	{ OV7725_GAM11        , 0xa4 },
	{ OV7725_GAM12        , 0xaf },
	{ OV7725_GAM13        , 0xc5 },
	{ OV7725_GAM14        , 0xd7 },
	{ OV7725_GAM15        , 0xe8 },
	{ OV7725_SLOP         , 0x20 },
	{ OV7725_LC_RADI      , 0x00 },
	{ OV7725_LC_COEF      , 0x13 },
	{ OV7725_LC_XC        , 0x08 },
	{ OV7725_LC_COEFB     , 0x14 },
	{ OV7725_LC_COEFR     , 0x17 },
	{ OV7725_LC_CTR       , 0x05 },
	{ OV7725_BDBase       , 0x99 },
	{ OV7725_BDMStep      , 0x03 },
	{ OV7725_SDE          , 0x04 },
	{ OV7725_BRIGHT       , 0x00 },
	{ OV7725_CNST         , TWOchNum },	//二值化阈值设置；57
	{ OV7725_SIGN         , 0x06 },
	{ OV7725_UVADJ0       , 0x11 },
	{ OV7725_UVADJ1       , 0x02 },
};

void delay_us(unsigned int t)
{
	int i=0;
	for( i=0;i<t;i++)
	{
		int a=40;
	//	int a=0;
		while(a--);
	}
	//i++;
}

void CAMERA_I2C_MspInit() {
	GPIO_InitTypeDef  GPIO_InitStruct;

	/*** Configure the GPIOs ***/
	/* Enable GPIO clock */
	CAMERA_I2C_SCL_SDA_GPIO_CLK_ENABLE();

	/* Configure I2C Tx as alternate function */
	GPIO_InitStruct.Pin = CAMERA_I2C_SCL_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
	GPIO_InitStruct.Alternate = CAMERA_I2C_SCL_SDA_AF;
	HAL_GPIO_Init(CAMERA_I2C_SCL_SDA_GPIO_PORT, &GPIO_InitStruct);

	/* Configure I2C Rx as alternate function */
	GPIO_InitStruct.Pin = CAMERA_I2C_SDA_PIN;
	HAL_GPIO_Init(CAMERA_I2C_SCL_SDA_GPIO_PORT, &GPIO_InitStruct);

	/*** Configure the I2C peripheral ***/
	/* Enable I2C clock */
	CAMERA_I2C_CLK_ENABLE();

	/* Force the I2C peripheral clock reset */
	//CAMERA_I2C_FORCE_RESET();

	///* Release the I2C peripheral clock reset */
	//CAMERA_I2C_RELEASE_RESET();

	///* Set priority and enable I2Cx event Interrupt */
	//HAL_NVIC_SetPriority(CAMERA_I2C_EV_IRQn, 5, 0);
	//HAL_NVIC_EnableIRQ(CAMERA_I2C_EV_IRQn);

	///* Set priority and enable I2Cx error Interrupt */
	//HAL_NVIC_SetPriority(CAMERA_I2C_ER_IRQn, 5, 0);
	//HAL_NVIC_EnableIRQ(CAMERA_I2C_ER_IRQn);
}

void CAMERA_I2C_Init() {
	if (HAL_I2C_GetState(&camera_I2c) == HAL_I2C_STATE_RESET)
	{
		camera_I2c.Instance = CAMERA_I2C;
		camera_I2c.Init.ClockSpeed = CAMERA_I2C_SPEED;
		camera_I2c.Init.DutyCycle = I2C_DUTYCYCLE_2;
		camera_I2c.Init.OwnAddress1 = 0;
		camera_I2c.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
		camera_I2c.Init.DualAddressMode = I2C_DUALADDRESS_DISABLED;
		camera_I2c.Init.OwnAddress2 = 0;
		camera_I2c.Init.GeneralCallMode = I2C_GENERALCALL_DISABLED;
		camera_I2c.Init.NoStretchMode = I2C_NOSTRETCH_DISABLED;

		/* Init the I2C */
		CAMERA_I2C_MspInit();
		HAL_I2C_Init(&camera_I2c);
	}
}

static void CAMERA_I2C_Error(uint8_t Addr)
{
	/* De-initialize the IOE comunication BUS */
	HAL_I2C_DeInit(&camera_I2c);

	/* Re-Initiaize the IOE comunication BUS */
	CAMERA_I2C_Init();
}

uint8_t CAMERA_I2C_Read(uint8_t Reg) {
	HAL_StatusTypeDef status = HAL_OK;
	uint8_t Value = 0;

	status = HAL_I2C_Mem_Read(&camera_I2c, CAMERA_READ_ADDR, Reg, I2C_MEMADD_SIZE_8BIT, &Value, 1, I2C_TIMEOUT);

	/* Check the communication status */
	if (status != HAL_OK)
	{
		/* Execute user timeout callback */
		CAMERA_I2C_Error(CAMERA_READ_ADDR);
	}

	return Value;
}

void CAMERA_I2C_Write(uint8_t Reg, uint8_t Value) {
	HAL_StatusTypeDef status = HAL_OK;

	status = HAL_I2C_Mem_Write(&camera_I2c, CAMERA_WRITE_ADDR, (uint16_t)Reg, I2C_MEMADD_SIZE_8BIT, &Value, 1, I2C_TIMEOUT);

	/* Check the communication status */
	if (status != HAL_OK)
	{
		/* I2C error occured */
		CAMERA_I2C_Error(CAMERA_WRITE_ADDR);
	}
}



void SCCB_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStruct;
	__GPIOB_CLK_ENABLE();

	GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_5;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

void SCCB_SID_OUT(void)//设置SCCB_SID为输出
{
	GPIO_InitTypeDef  GPIO_InitStruct;

	GPIO_InitStruct.Pin =  GPIO_PIN_5;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}
void SCCB_SID_IN(void)//设置SCCB_SID为输入
{
	GPIO_InitTypeDef  GPIO_InitStruct;

	GPIO_InitStruct.Pin = GPIO_PIN_5;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

void SCCB_Start(void)
{
	SCCB_SID_H();     //数据线高电平
	delay_us(50);
	SCCB_SIC_H();	   //在时钟线高的时候数据线由高至低
	delay_us(50);
	SCCB_SID_L();
	delay_us(50);
	SCCB_SIC_L();	 //数据线恢复低电平，单操作函数必要
	delay_us(50);
}

void SCCB_Stop(void)
{
	SCCB_SID_L();
	delay_us(50);
	SCCB_SIC_H();
	delay_us(50);
	SCCB_SID_H();
	delay_us(50);
}

void noAck(void)
{
	SCCB_SID_H();
	delay_us(50);
	SCCB_SIC_H();
	delay_us(50);
	SCCB_SIC_L();
	delay_us(50);
	SCCB_SID_L();
	delay_us(50);
}

uint8_t SCCB_Write(uint8_t m_data)
{
	uint8_t j, tem;

	for (j = 0; j < 8; j++) //循环8次发送数据
	{
		if ((m_data << j) & 0x80)SCCB_SID_H();
		else SCCB_SID_L();
		delay_us(50);
		SCCB_SIC_H();
		delay_us(50);
		SCCB_SIC_L();
		delay_us(50);
	}
	delay_us(10);
	SCCB_DATA_IN;
	delay_us(50);
	SCCB_SIC_H();
	delay_us(10);
	if (SCCB_SID_STATE)tem = 0;//SDA=1发送失败
	else tem = 1;//SDA=0发送成功，返回1
	SCCB_SIC_L();
	delay_us(50);
	SCCB_DATA_OUT;
	return tem;
}

uint8_t SCCB_Read(void)
{
	uint8_t read, j;
	read = 0x00;

	SCCB_DATA_IN;
	delay_us(50);
	for (j = 8; j > 0; j--) //循环8次接收数据
	{
		delay_us(50);
		SCCB_SIC_H();
		delay_us(50);
		read = read << 1;
		if (SCCB_SID_STATE)read = read + 1;
		SCCB_SIC_L();
		delay_us(50);
	}
	SCCB_DATA_OUT;
	return read;
}

//写OV7670寄存器
uint8_t OV_WriteReg(uint8_t regID, uint8_t regDat)
{
	SCCB_Start();//发送SCCB 总线开始传输命令
	if (SCCB_Write(0x42) == 0)//写地址
	{
		SCCB_Stop();//发送SCCB 总线停止传输命令
		return 1;//错误返回
	}
	delay_us(10);
	if (SCCB_Write(regID) == 0)//积存器ID
	{
		SCCB_Stop();//发送SCCB 总线停止传输命令
		return 2;//错误返回
	}
	delay_us(10);
	if (SCCB_Write(regDat) == 0)//写数据到积存器
	{
		SCCB_Stop();//发送SCCB 总线停止传输命令
		return 3;//错误返回
	}
	SCCB_Stop();//发送SCCB 总线停止传输命令	
	return 0;//成功返回
}

//读OV7660寄存器
uint8_t OV_ReadReg(uint8_t regID, uint8_t *regDat)
{
	//通过写操作设置寄存器地址
	SCCB_Start();
	if (SCCB_Write(0x42) == 0)//写地址
	{
		SCCB_Stop();//发送SCCB 总线停止传输命令
		return 1;//错误返回
	}
	delay_us(10);
	if (SCCB_Write(regID) == 0)//积存器ID
	{
		SCCB_Stop();//发送SCCB 总线停止传输命令
		return 2;//错误返回
	}
	SCCB_Stop();//发送SCCB 总线停止传输命令	
	delay_us(10);
	//设置寄存器地址后，才是读
	SCCB_Start();
	if (SCCB_Write(0x43) == 0)//读地址
	{
		SCCB_Stop();//发送SCCB 总线停止传输命令
		return 3;//错误返回
	}
	delay_us(10);
	*regDat = SCCB_Read();//返回读到的值
	noAck();//发送NACK命令
	SCCB_Stop();//发送SCCB 总线停止传输命令
	return 0;//成功返回
}

void OV_Reset(void)
{
	OV_WriteReg(0x12, 0x80);
}

uint8_t OV_ReadID(void)
{
	uint8_t temp;
	OV_ReadReg(0x0a, &temp);
	return temp;
}

void OV7670_config_window(uint16_t startx, uint16_t starty, uint16_t width, uint16_t height) {
		uint16_t endx = (startx + width * 2) % 784; uint16_t endy = (starty + height * 2); uint8_t x_reg, y_reg; uint8_t state, temp;
		state = OV_ReadReg(0x32, &x_reg);
		x_reg &= 0xC0; 
		state = OV_ReadReg(0x03, &y_reg);
		y_reg &= 0xF0;
		//设置 HREF 
		temp = x_reg|((endx&0x7)<<3)|(startx&0x7); 
		state = OV_WriteReg(0x32, temp );
		temp = (startx&0x7F8)>>3; 
		state = OV_WriteReg(0x17, temp );
		temp = (endx&0x7F8)>>3; 
		state = OV_WriteReg(0x18, temp );
		//设置 VREF 
		temp = y_reg|((endy&0x3)<<2)|(starty&0x3); 
		state = OV_WriteReg(0x03, temp );
		temp = (starty&0x3FC)>>2; 
		state = OV_WriteReg(0x19, temp );
		temp = (endy&0x3FC)>>2; 
		state = OV_WriteReg(0x1A, temp );

}

uint8_t OV7670_Init(void)
{
	uint8_t i;
	Cam_Init();
	SCCB_Init();
	OV_Reset();
	
	HAL_Delay(5);
	for (i = 0; i < OV7670_REG_NUM; i++)
	{
		if (OV_WriteReg(OV7670_reg[i][0], OV7670_reg[i][1]))return 1;
	}
	return 0;
}

uint8_t OV7725_Init(void)
{
	uint8_t i;
	Cam_Init();
	SCCB_Init();
	OV_Reset();

	HAL_Delay(5);
	for (i = 0; i < OV7725_REG_NUM; i++)
	{
		if (OV_WriteReg(ov7725_eagle_reg_one[i][0], ov7725_eagle_reg_one[i][1]))return 1;
	}
	return 0;
}

static void DCMI_MspInit(void)
{
	static DMA_HandleTypeDef hdma;
	GPIO_InitTypeDef GPIO_Init_Structure;
	DCMI_HandleTypeDef *hdcmi = &hdcmi_camera;

	/*** Enable peripherals and GPIO clocks ***/
	/* Enable DCMI clock */
	__DCMI_CLK_ENABLE();

	/* Enable DMA2 clock */
	__DMA2_CLK_ENABLE();

	/* Enable GPIO clocks */
	__GPIOA_CLK_ENABLE();
	__GPIOB_CLK_ENABLE();
	__GPIOC_CLK_ENABLE();

	/*** Configure the GPIO ***/
	/* Configure DCMI GPIO as alternate function */
	GPIO_Init_Structure.Pin = GPIO_PIN_4 | GPIO_PIN_6;
	GPIO_Init_Structure.Mode = GPIO_MODE_AF_PP;
	GPIO_Init_Structure.Pull = GPIO_PULLUP;
	GPIO_Init_Structure.Speed = GPIO_SPEED_HIGH;
	GPIO_Init_Structure.Alternate = GPIO_AF13_DCMI;
	HAL_GPIO_Init(GPIOA, &GPIO_Init_Structure);

	GPIO_Init_Structure.Pin = GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9;
	GPIO_Init_Structure.Mode = GPIO_MODE_AF_PP;
	GPIO_Init_Structure.Pull = GPIO_PULLUP;
	GPIO_Init_Structure.Speed = GPIO_SPEED_HIGH;
	GPIO_Init_Structure.Alternate = GPIO_AF13_DCMI;
	HAL_GPIO_Init(GPIOB, &GPIO_Init_Structure);

	GPIO_Init_Structure.Pin = GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_11;
	GPIO_Init_Structure.Mode = GPIO_MODE_AF_PP;
	GPIO_Init_Structure.Pull = GPIO_PULLUP;
	GPIO_Init_Structure.Speed = GPIO_SPEED_HIGH;
	GPIO_Init_Structure.Alternate = GPIO_AF13_DCMI;
	HAL_GPIO_Init(GPIOC, &GPIO_Init_Structure);

	/*** Configure the DMA streams ***/
	/* Configure the DMA handler for Transmission process */
	hdma.Init.Channel = DMA_CHANNEL_1;
	hdma.Init.Direction = DMA_PERIPH_TO_MEMORY;
	hdma.Init.PeriphInc = DMA_PINC_DISABLE;
	hdma.Init.MemInc = DMA_MINC_ENABLE;
	hdma.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
	hdma.Init.MemDataAlignment = DMA_PDATAALIGN_WORD;
	hdma.Init.Mode = DMA_CIRCULAR;
	hdma.Init.Priority = DMA_PRIORITY_HIGH;
	hdma.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
	hdma.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
	hdma.Init.MemBurst = DMA_MBURST_SINGLE;
	hdma.Init.PeriphBurst = DMA_PBURST_SINGLE;

	hdma.Instance = DMA2_Stream1;

	/* Associate the initialized DMA handle to the DCMI handle */
	__HAL_LINKDMA(hdcmi, DMA_Handle, hdma);

	/*** Configure the NVIC for DCMI and DMA ***/
	/* NVIC configuration for DCMI transfer complete interrupt */
	HAL_NVIC_SetPriority(DCMI_IRQn, 5, 0);
	HAL_NVIC_EnableIRQ(DCMI_IRQn);

	/* NVIC configuration for DMA2 transfer complete interrupt */
	HAL_NVIC_SetPriority(DMA2_Stream1_IRQn, 5, 0);
	HAL_NVIC_EnableIRQ(DMA2_Stream1_IRQn);

	/* Configure the DMA stream */
	HAL_DMA_Init(hdcmi->DMA_Handle);
}

void Cam_Init() {
	DCMI_HandleTypeDef *phdcmi;

	/* Get the DCMI handle structure */
	phdcmi = &hdcmi_camera;

	/*** Configures the DCMI to interface with the Camera module ***/
	/* DCMI configuration */
	phdcmi->Init.CaptureRate = DCMI_CR_ALL_FRAME;
	phdcmi->Init.HSPolarity = DCMI_HSPOLARITY_HIGH;
	phdcmi->Init.SynchroMode = DCMI_SYNCHRO_HARDWARE;
	phdcmi->Init.VSPolarity = DCMI_VSPOLARITY_HIGH;
	phdcmi->Init.ExtendedDataMode = DCMI_EXTEND_DATA_8B;
	phdcmi->Init.PCKPolarity = DCMI_PCKPOLARITY_RISING;
	phdcmi->Instance = DCMI;

	/* DCMI Initialization */
	DCMI_MspInit();
	HAL_DCMI_Init(phdcmi);
}

void BSP_CAMERA_ContinuousStart(uint32_t *buff)
{
	/* Start the Camera capture */
	HAL_DCMI_Start_DMA(&hdcmi_camera, DCMI_MODE_CONTINUOUS, (uint32_t)buff, 120*160);
}

void BSP_CAMERA_SnapshotStart(uint8_t *buff)
{
	/* Start the Camera capture */
	HAL_DCMI_Start_DMA(&hdcmi_camera, DCMI_MODE_SNAPSHOT, (uint32_t)buff, 9600);
}

/**
* @brief  Suspends the Camera capture.
*/
void BSP_CAMERA_Suspend(void)
{
	/* Suspend the Camera Capture */
	HAL_DCMI_Suspend(&hdcmi_camera);
}

/**
* @brief  Resumes the Camera capture.
*/
void BSP_CAMERA_Resume(void)
{
	/* Start the Camera Capture */
	HAL_DCMI_Resume(&hdcmi_camera);
}


void CAMERA_I2C_test()
{
	//CAMERA_I2C_Init();
	//CAMERA_I2C_Write(0x12, 0x80);
	/*test_val = CAMERA_I2C_Read(0x0a);*/

	
	test_val = 1;
	OV7725_Init();
	//SCCB_Init();
	test_val = OV_ReadID();
	//HAL_Delay(1);
	//BSP_CAMERA_ContinuousStart(&camera_buffer[0][0]);
	//HAL_Delay(1000);
	//BSP_CAMERA_Suspend();
}

int Line_num = 0;
int get_line = 0;
int Frame = 0;
void HAL_DCMI_LineEventCallback(DCMI_HandleTypeDef *hdcmi)
{
	Line_num++;
}

void HAL_DCMI_FrameEventCallback(DCMI_HandleTypeDef *hdcmi)
{
	get_line = Line_num;
	Line_num = 0;
	Frame++;
	//oled_camera_display(camera_buffer);
}

void BSP_CAMERA_IRQHandler(void)
{
	HAL_DCMI_IRQHandler(&hdcmi_camera);
}

/**
* @brief  Handles DMA interrupt request.
*/
void BSP_CAMERA_DMA_IRQHandler(void)
{
	HAL_DMA_IRQHandler(hdcmi_camera.DMA_Handle);
}


void DMA2_Stream1_IRQHandler(void)
{
	BSP_CAMERA_DMA_IRQHandler();
}

/**
* @brief  DCMI interrupt handler.
* @param  None
* @retval None
*/
void DCMI_IRQHandler(void)
{
	BSP_CAMERA_IRQHandler();
}