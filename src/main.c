#include "stm32f10x_lib.h"
#include "hw.h"
#include "uartio.h"

static void handleCommand(char c) {

   switch(c) {
      case 'a':
      case 'A':
         /* Single ADC measurement */
         printhex(getPower());
         print("\n");
         break;

      case 'c':
      case 'C':
         /* Continous ADC */
         print("Press any key to stop...\n");
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
         print("Power to RF-chain DISABLED.\n");
         break;

         /* Enable RF-chain power */
      case 'e':
      case 'E':
         setRFpowerState(1);
         print("Power to RF-chain ENABLED.\n");
         break;

         /* Set antenna switch */
      case '1':
         setAntennaSwitch(0);
         print("Antenna switch now LOW.\n");
         break;
      case '2':
         setAntennaSwitch(1);
         print("Antenna switch now HIGH.\n");
         break;

      case 'H':
      case 'h':
      case '?':
         /* Help */
         print(
               "a    Measure power\n"
               "c    Measure power continously\n"
               "d    Disable RF-chain power\n"
               "e    Enable RF-chain power\n"
               "1 2  Select power measurement antenna\n"
               "h    Show this message\n");
         break;

      default:
         /* Unknown */
         print("Unknown command, type 'h' for help.\n");
   }
}

__attribute__((noreturn))
static void commandLine(void) {
   char buf[3];

   /* Added for command echo on UART */
   buf[1] = '\n'; buf[2] = '\0';

   print("System ready.\n");

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
