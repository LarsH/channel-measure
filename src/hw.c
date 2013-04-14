#include "stm32f10x_lib.h"

/* Private variables */
GPIO_InitTypeDef GPIO_InitStructure;
ErrorStatus HSEStartUpStatus;
USART_InitTypeDef USART_InitStructure;

void initHardware(void) {
   /* Enable GPIOA and USART1 clock */
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1, ENABLE);

   /* Configure USART1_Tx as alternate function push-pull */
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
   GPIO_Init(GPIOA, &GPIO_InitStructure);

   /* Configure USART1_Rx as input floating */
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
   GPIO_Init(GPIOA, &GPIO_InitStructure);

   USART_StructInit(&USART_InitStructure);
   /* Standard init is 9600 baud, 8 bit, no parity */
   USART_Init(USART1, &USART_InitStructure);
   USART_Cmd(USART1, ENABLE);
}
