#include "stm32f10x_lib.h"

void print(char *s) {
   /** Prints the null-terminated string s to UART1.
     Blocking IO
     UART1 must be initialized
     */
   int i;
   for(i=0; s[i] != '\0'; i++) {
      USART_SendData(USART1, s[i]);
      while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == 0);
   }
}

int getchar(char *storeLocation) {
   /** Checks UART1 for received a received character
     UART1 must be initialized
     Returns zero if no data is received.
     Returns non-zero and stores the received byte at *storeLocation otherwise.
    */
   if(USART_GetFlagStatus(USART1, USART_FLAG_RXNE)) {
      /* Reading the data clears the RXNE-flag */
      *storeLocation = (char) USART_ReceiveData(USART1);
      return 1;
   }
   else {
      return 0;
   }
}
