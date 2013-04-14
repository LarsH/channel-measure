#include "stm32f10x_lib.h"
#include "hw.h"

/* Private variables */

/* Private function prototypes */
void Delay(vu32 nCount);

/* Private functions */
void Delay(vu32 nCount)
{
   for(; nCount != 0; nCount--);
}

int main(void)
{
   initHardware();
   while (1)
   {
      int i;
      const char *text = "Hello world!\r\n";
      for(i=0; i<14; i++) {
         USART_SendData(USART1, text[i]);
         while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == 0);
      }
   }
}
