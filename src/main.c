#include "stm32f10x_lib.h"

/* Private variables */
GPIO_InitTypeDef GPIO_InitStructure;
ErrorStatus HSEStartUpStatus;

/* Private function prototypes */
void Delay(vu32 nCount);

/* Private functions */
void Delay(vu32 nCount)
{
  for(; nCount != 0; nCount--);
}

int main(void)
{
  /* Enable GPIOA clock */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

  /* Configure PA.9 as Output push-pull */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  while (1)
  {
    /* Turn on led connected to PA.9 pin */
    GPIO_SetBits(GPIOA, GPIO_Pin_9);
    /* Insert delay */
    Delay(0xAFFFF);

    /* Turn off led connected to PA.9 pin */
    GPIO_ResetBits(GPIOA, GPIO_Pin_9);
    /* Insert delay */
    Delay(0xAFFFF);
  }
}
