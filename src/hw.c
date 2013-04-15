#include "stm32f10x_lib.h"
#include "uartio.h"

/* Private variables */
GPIO_InitTypeDef GPIO_InitStructure;
ErrorStatus HSEStartUpStatus;
USART_InitTypeDef USART_InitStructure;
ADC_InitTypeDef ADC_InitStructure;

void initHardware(void) {
   int i;

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
   USART_InitStructure.USART_BaudRate = 115200;
   USART_Init(USART1, &USART_InitStructure);
   USART_Cmd(USART1, ENABLE);
   print("UART initialized\r\n");

   /* Initialize ADC */
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
   ADC_StructInit(&ADC_InitStructure);
   ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
   ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;

   ADC_Init(ADC1, &ADC_InitStructure);
   ADC_Cmd(ADC1, ENABLE);

   print("Starting ADC calibration.");
   ADC_StartCalibration(ADC1);

   i = 0;
   while(ADC_GetCalibrationStatus(ADC1) == SET) {
      i++;
      /* Typically calibration is done after four checks */
      if(i>100) {
         print("Warning: ADC calibration failed!\r\n");
         break;
      }
   }
   print("\r\nADC calibration done.\r\n");

   /* uC pin 10, PA0, should have alternate function ADC12_IN0 */
   ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_7Cycles5);
   ADC_SoftwareStartConvCmd(ADC1, ENABLE);

}

unsigned int getPower(void) {
   return ADC_GetConversionValue(ADC1);
}
