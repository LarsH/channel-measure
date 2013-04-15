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
   initHardware();
   for(;;)
   {
      printhex(getPower());
      print("\r");
   }
}
