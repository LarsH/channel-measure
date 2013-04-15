#include "stm32f10x_lib.h"
#include "hw.h"
#include "uartio.h"

void handleCommand(char c) {

   switch(c) {
      case 'a':
      case 'A':
         /* Single ADC measurement */
         printhex(getPower());
         print("\r\n");
         break;

      case 'c':
      case 'C':
         /* Continous ADC */
         print("Press any key to stop...\r\n");
         while(getchar(&c) == 0) {
            printhex(getPower());
            print("\r");
         }
         print("\n");
         break;

      case 'H':
      case 'h':
      case '?':
         /* Help */
         print(
               "a    Measure power\r\n"
               "c    Measure power continously\r\n"
               "h    Show this message\r\n");
         break;

      default:
         /* Unknown */
         print("Unknown command, type 'h' for help.\r\n");
   }
}

void commandLine(void) {
   char buf[4];

   /* Added for command echo on UART */
   buf[1] = '\r'; buf[2] = '\n'; buf[3] = '\0';

   print("System ready.\r\n");

   /* Infinite command loop */
   for(;;) {
      print(">>> ");
      while(getchar(&(buf[0])) == 0)
         ;

      print(buf); /* Echo command */

      handleCommand(buf[0]);
   }
}

int main(void)
{
   initHardware();
   commandLine();
}
