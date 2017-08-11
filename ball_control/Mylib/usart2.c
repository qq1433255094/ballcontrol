//#include "usart2.h"
//
//USART_HandleTypeDef husart2;
//uint8_t receive2[10],transmit2[10];
//
///**
//  * @brief 串口2初始化
//  * @param 无
//  * @note 无
//  * @retval 无
//  */
//void usart2_init(void)
//{
//	GPIO_InitTypeDef gpio_init;
//	
//	__HAL_RCC_USART2_CLK_ENABLE();
//	__HAL_RCC_GPIOD_CLK_ENABLE();
//	
//	gpio_init.Alternate=GPIO_AF7_USART2;
//	gpio_init.Pin=GPIO_PIN_5|GPIO_PIN_6;
//	gpio_init.Mode=GPIO_MODE_AF_PP;
//	gpio_init.Speed=GPIO_SPEED_FREQ_HIGH;
//	gpio_init.Pull=GPIO_PULLUP;
//	HAL_GPIO_Init(GPIOD,&gpio_init);
//	
//	husart2.Init.BaudRate=115200;
//	husart2.Init.WordLength=USART_WORDLENGTH_8B;
//	husart2.Init.StopBits=USART_STOPBITS_1;
//	husart2.Init.Parity=USART_PARITY_NONE;
//	husart2.Init.Mode=USART_MODE_TX_RX;
//	husart2.Instance=USART2;
//
//	HAL_USART_Init(&husart2);
//	
//}
//
///**
//  * @brief 串口2发送字符串
//  * @param *b：字符串数组指针
//  * @note 无
//  * @retval 无
//  */
//void usart2_send(char *b)
//{
//	char len=strlen(b);
//	if(*(b+len-1)==0)
//		len--;
//	HAL_USART_Transmit(&husart2,(uint8_t *)b,len,100);
//}
//
//static char buffer[100];
//static char stradd[10];
//
//void usart2_ptint(char *str,int data,int mode)
//{
//#ifdef USE_PRINTF
//	usart2_send(str);
//	itoa((int)data, buffer, mode);
//	usart2_send(buffer);
//	usart2_send("\n");
//#endif // USE_PRINTF
//}
//
//int tenpow(int len)
//{
//	int data = 1;
//	while (len--)
//		data = 10*data;
//	return data;
//}
//
//void usart2_ptintf(char *str, float data, int len)
//{
//#ifdef USE_PRINTF
//	
//	usart2_send(str);
//	itoa((int)data, buffer, 10);
//	if (len)
//	{
//		strncat(buffer, ".", 1);
//		itoa((int)((data - (int)data) * tenpow(len)), stradd, 10);
//		strncat(buffer, stradd, 10);
//	}
//	usart2_send(buffer);
//	usart2_send("\n");
//	
//#endif // USE_PRINTF
//}
//
//void USART2_IRQHandler(void)
//{
//	
//}
//
//




#include "usart2.h"



/**
* @brief 串口2初始化
* @param 无
* @note 无
* @retval 无
*/

__IO ITStatus UartReady = RESET;

UART_HandleTypeDef huart2;
DMA_HandleTypeDef  hdma_tx;
DMA_HandleTypeDef  hdma_rx;

void uart2_Config(uint32_t bRate)
{
	__HAL_RCC_USART2_CLK_ENABLE();

	huart2.Instance = USART2;

	huart2.Init.BaudRate = bRate;
	huart2.Init.WordLength = UART_WORDLENGTH_8B;
	huart2.Init.StopBits = UART_STOPBITS_1;
	huart2.Init.Parity = UART_PARITY_NONE;
	huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart2.Init.Mode = UART_MODE_TX_RX;
	huart2.Init.OverSampling = UART_OVERSAMPLING_16;

	if (HAL_UART_Init(&huart2) != HAL_OK)
	{

	}

}


void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
	GPIO_InitTypeDef GPIO;

	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_DMA1_CLK_ENABLE();

	/* UART TX GPIO pin configuration  */
	GPIO.Pin = GPIO_PIN_5 | GPIO_PIN_6;
	GPIO.Mode = GPIO_MODE_AF_PP;
	GPIO.Pull = GPIO_NOPULL;
	GPIO.Speed = GPIO_SPEED_FAST;
	GPIO.Alternate = GPIO_AF7_USART2;
	HAL_GPIO_Init(GPIOD, &GPIO);


	/* Configure the DMA handler for Transmission process */
	hdma_tx.Instance = DMA1_Stream6;

	hdma_tx.Init.Channel = DMA_CHANNEL_4;
	hdma_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
	hdma_tx.Init.PeriphInc = DMA_PINC_DISABLE;
	hdma_tx.Init.MemInc = DMA_MINC_ENABLE;
	hdma_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
	hdma_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
	hdma_tx.Init.Mode = DMA_CIRCULAR;
	hdma_tx.Init.Priority = DMA_PRIORITY_LOW;
	hdma_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
	hdma_tx.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
	hdma_tx.Init.MemBurst = DMA_MBURST_INC4;
	hdma_tx.Init.PeriphBurst = DMA_PBURST_INC4;

	HAL_DMA_Init(&hdma_tx);

	/* Associate the initialized DMA handle to the the UART handle */
	__HAL_LINKDMA(huart, hdmatx, hdma_tx);

	/* Configure the DMA handler for Transmission process */
	hdma_rx.Instance = DMA1_Stream5;

	hdma_rx.Init.Channel = DMA_CHANNEL_4;
	hdma_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
	hdma_rx.Init.PeriphInc = DMA_PINC_DISABLE;
	hdma_rx.Init.MemInc = DMA_MINC_ENABLE;
	hdma_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
	hdma_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
	hdma_rx.Init.Mode = DMA_CIRCULAR;
	hdma_rx.Init.Priority = DMA_PRIORITY_HIGH;
	hdma_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
	hdma_rx.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
	hdma_rx.Init.MemBurst = DMA_MBURST_INC4;
	hdma_rx.Init.PeriphBurst = DMA_PBURST_INC4;

	HAL_DMA_Init(&hdma_rx);

	/* Associate the initialized DMA handle to the the UART handle */
	__HAL_LINKDMA(huart, hdmarx, hdma_rx);

	/*##-4- Configure the NVIC for DMA #########################################*/
	/* NVIC configuration for DMA transfer complete interrupt (USARTx_TX) */
	HAL_NVIC_SetPriority(DMA1_Stream6_IRQn, 0, 1);
	HAL_NVIC_EnableIRQ(DMA1_Stream6_IRQn);

	/* NVIC configuration for DMA transfer complete interrupt (USARTx_RX) */
	HAL_NVIC_SetPriority(DMA1_Stream5_IRQn, 0, 1);
	HAL_NVIC_EnableIRQ(DMA1_Stream5_IRQn);

	/* NVIC configuration for USART TC interrupt */
	HAL_NVIC_SetPriority(USART2_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(USART2_IRQn);
}

void Send_data(uint8_t *pData)
{

	char len = strlen((char *)pData);
	if (*(pData + len - 1) == 0)
		len--;

	//HAL_UART_Transmit(&huart2, pData,  len, 100);
	HAL_UART_Transmit_DMA(&huart2, pData, len);

}


/**
* @brief  Tx Transfer completed callback
* @param  UartHandle: UART handle.
* @note   This example shows a simple way to report end of DMA/IT Tx transfer, and
*         you can add your own implementation.
* @retval None
*/
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *UartHandle)
{


}


/**
* @brief  Rx Transfer completed callback
* @param  UartHandle: UART handle
* @note   This example shows a simple way to report end of DMA Rx transfer, and
*         you can add your own implementation.
* @retval None
*/
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartHandle)
{



}



