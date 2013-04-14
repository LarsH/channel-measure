#include "stm32f10x_lib.h"
#include "hw.h"
#include "uartio.h"

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
   int i = 0;
   char buf[2];
   buf[1] = '\0';
   initHardware();
   while (1)
   {
      while(getchar(buf) == 0)
         ;

      printhex(i);
      i = i+1;
      print(" Got char: ");
      print(buf);
      print("\r\n");
   }
}
