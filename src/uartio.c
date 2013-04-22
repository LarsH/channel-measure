#include "stm32f10x_lib.h"

void print(char *s) {
   /** Prints the null-terminated string s to UART1.
     For every '\n' character, "\r\n" is printed
     Blocking IO
     UART1 must be initialized
    */
   int i;
   for(i=0; s[i] != '\0'; i++) {
      if(s[i] == '\n') {
         USART_SendData(USART1, (u16) '\r');
         while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == 0);
      }
      USART_SendData(USART1, (u16) s[i]);
      while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == 0);
   }
}

int getuartchar(char *storeLocation)
{
   /** Checks UART1 for received a received character
     UART1 must be initialized
     Returns zero and stores '\0' at *storeLocation if no data is received.
     Returns non-zero and stores the received byte at *storeLocation otherwise.
    */
   if(USART_GetFlagStatus(USART1, USART_FLAG_RXNE) != 0) {
      /* Reading the data clears the RXNE-flag */
      *storeLocation = (char) USART_ReceiveData(USART1);
      return 1;
   }
   else {
      *storeLocation = (char) '\0';
      return 0;
   }
}

void printhex(unsigned int h) {
   /* Prints the 32 bit integer h in hexadecimal on UART1
      Blocking IO
      UART1 must be initialized
    */
   unsigned int i;
   char buf[2];
   char * digits = "0123456789abcdef";
   buf[1] = '\0';
   for(i=0U; i<8U; i++) {
      buf[0] = digits[0xf & (h>>28U)];
      print(buf);
      h <<= 4U;
   }
}
