#include "stm32f10x_lib.h"
#include "hw.h"
#include "uartio.h"

static void handleCommand(char c) {

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
         while(getuartchar(&c) == 0) {
            printhex(getPower());
            print("\r");
         }
         print("\n");
         break;

         /* Disable RF-chain power */
      case 'd':
      case 'D':
         setRFpowerState(0);
         print("Power to RF-chain DISABLED.\r\n");
         break;

         /* Enable RF-chain power */
      case 'e':
      case 'E':
         setRFpowerState(1);
         print("Power to RF-chain ENABLED.\r\n");
         break;

         /* Set antenna switch */
      case '1':
         setAntennaSwitch(0);
         print("Antenna switch now LOW.\r\n");
         break;
      case '2':
         setAntennaSwitch(1);
         print("Antenna switch now HIGH.\r\n");
         break;

      case 'H':
      case 'h':
      case '?':
         /* Help */
         print(
               "a    Measure power\r\n"
               "c    Measure power continously\r\n"
               "d    Disable RF-chain power\r\n"
               "e    Enable RF-chain power\r\n"
               "1 2  Select power measurement antenna\r\n"
               "h    Show this message\r\n");
         break;

      default:
         /* Unknown */
         print("Unknown command, type 'h' for help.\r\n");
   }
}

__attribute__((noreturn))
static void commandLine(void) {
   char buf[4];

   /* Added for command echo on UART */
   buf[1] = '\r'; buf[2] = '\n'; buf[3] = '\0';

   print("System ready.\r\n");

   /* Infinite command loop */
   for(;;) {
      print(">>> ");
      while(getuartchar(&(buf[0])) == 0)
         ;

      print(buf); /* Echo command */

      handleCommand(buf[0]);
   }
}

void main(void)
{
   initHardware();
   commandLine();
}
