#include "usart3.h"
#include "arm_math.h"
#include "can2.h"

UART_HandleTypeDef huart3;
uint8_t receive3[20], transmit3[20];
uint8_t page_fir[] = "page 2xxx";
uint8_t page_sec[] = "page 3xxx";
uint8_t area[4];



/**
* @brief ³õÊ¼»¯´®¿Ú3
* @param none
* @note none
* @retval none
*/
void usart3_init(void)
{
	GPIO_InitTypeDef gpio_init;


	HAL_NVIC_SetPriority(USART3_IRQn, 4, 0);
	HAL_NVIC_EnableIRQ(USART3_IRQn);

	page_fir[6] = 0xff;
	page_fir[7] = 0xff;
	page_fir[8] = 0xff;

	page_sec[6] = 0xff;
	page_sec[7] = 0xff;
	page_sec[8] = 0xff;

	__HAL_RCC_USART3_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

	gpio_init.Alternate = GPIO_AF7_USART3;
	gpio_init.Pin = GPIO_PIN_10 | GPIO_PIN_11;
	gpio_init.Mode = GPIO_MODE_AF_PP;
	gpio_init.Speed = GPIO_SPEED_FREQ_HIGH;
	gpio_init.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(GPIOB, &gpio_init);

	huart3.Init.BaudRate = 9600;
	huart3.Init.WordLength = USART_WORDLENGTH_8B;
	huart3.Init.StopBits = USART_STOPBITS_1;
	huart3.Init.Parity = USART_PARITY_NONE;
	huart3.Init.Mode = USART_MODE_TX_RX;
	huart3.Instance = USART3;

	HAL_UART_Init(&huart3);
	HAL_UART_Receive_IT(&huart3, receive3, 4);
}


void usart3_send(const char *b)
{
	char len = strlen(b);
	HAL_UART_Transmit(&huart3, (uint8_t *)b, len, len * 2);
}

void USART3_IRQHandler(void)
{
	
	HAL_UART_IRQHandler(&huart3);
}


int8_t c;
void UART3_Handler(void)
{
	for (c = 0; c < 4; c++)
	{
		area[c] = receive3[c];
	}

	HAL_UART_Receive_IT(&huart3, receive3, 4);
}


