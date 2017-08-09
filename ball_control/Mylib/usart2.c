#include "usart2.h"

USART_HandleTypeDef husart2;
uint8_t receive2[10],transmit2[10];

/**
  * @brief ����2��ʼ��
  * @param ��
  * @note ��
  * @retval ��
  */
void usart2_init(void)
{
	GPIO_InitTypeDef gpio_init;
	
	__HAL_RCC_USART2_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
	
	gpio_init.Alternate=GPIO_AF7_USART2;
	gpio_init.Pin=GPIO_PIN_5|GPIO_PIN_6;
	gpio_init.Mode=GPIO_MODE_AF_PP;
	gpio_init.Speed=GPIO_SPEED_FREQ_HIGH;
	gpio_init.Pull=GPIO_PULLUP;
	HAL_GPIO_Init(GPIOD,&gpio_init);
	
	husart2.Init.BaudRate=115200;
	husart2.Init.WordLength=USART_WORDLENGTH_8B;
	husart2.Init.StopBits=USART_STOPBITS_1;
	husart2.Init.Parity=USART_PARITY_NONE;
	husart2.Init.Mode=USART_MODE_TX_RX;
	husart2.Instance=USART2;

	HAL_USART_Init(&husart2);
	
}

/**
  * @brief ����2�����ַ���
  * @param *b���ַ�������ָ��
  * @note ��
  * @retval ��
  */
void usart2_send(char *b)
{
	char len=strlen(b);
	if(*(b+len-1)==0)
		len--;
	HAL_USART_Transmit(&husart2,(uint8_t *)b,len,100);
}

static char buffer[100];
static char stradd[10];

void usart2_ptint(char *str,int data,int mode)
{
#ifdef USE_PRINTF
	usart2_send(str);
	itoa((int)data, buffer, mode);
	usart2_send(buffer);
	usart2_send("\n");
#endif // USE_PRINTF
}

int tenpow(int len)
{
	int data = 1;
	while (len--)
		data = 10*data;
	return data;
}

void usart2_ptintf(char *str, float data, int len)
{
#ifdef USE_PRINTF
	
	usart2_send(str);
	itoa((int)data, buffer, 10);
	if (len)
	{
		strncat(buffer, ".", 1);
		itoa((int)((data - (int)data) * tenpow(len)), stradd, 10);
		strncat(buffer, stradd, 10);
	}
	usart2_send(buffer);
	usart2_send("\n");
	
#endif // USE_PRINTF
}

void USART2_IRQHandler(void)
{
	
}


