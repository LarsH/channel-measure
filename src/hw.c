#include "stm32f10x_lib.h"
#include "hw.h"
#include "uartio.h"

/* Private Defines */
#define RF_POWER_PIN       GPIO_Pin_2
#define ANTENNA_SWITCH_PIN GPIO_Pin_7

/* Private variables */
static GPIO_InitTypeDef GPIO_InitStructure;
static USART_InitTypeDef USART_InitStructure;
static ADC_InitTypeDef ADC_InitStructure;

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
   print("UART initialized\n");

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
         print("Warning: ADC calibration failed!\n");
         break;
      }
   }
   print("\nADC calibration done.\n");

   /* uC pin 10, PA0, should have alternate function ADC12_IN0 */
   ADC_RegularChannelConfig(ADC1, ADC_Channel_0, (u8) 1, ADC_SampleTime_7Cycles5);
   ADC_SoftwareStartConvCmd(ADC1, ENABLE);

   /* Configure RF-chain control pins */
   GPIO_InitStructure.GPIO_Pin = RF_POWER_PIN | ANTENNA_SWITCH_PIN;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
   GPIO_Init(GPIOA, &GPIO_InitStructure);

}

unsigned int getPower(void) {
   return ADC_GetConversionValue(ADC1);
}

void setRFpowerState(int p) {
   /* Disables the power to the RF-chain if p is zero, enables the power
      otherwise. */
   if(p != 0) {
      GPIO_SetBits(GPIOA, RF_POWER_PIN);
   }
   else {
      GPIO_ResetBits(GPIOA, RF_POWER_PIN);
   }
}

void setAntennaSwitch(int t) {
   /* Sets the antenna switch high if t is non-zero; sets it low otherwise */
   if(t != 0) {
      GPIO_SetBits(GPIOA, ANTENNA_SWITCH_PIN);
   }
   else {
      GPIO_ResetBits(GPIOA, ANTENNA_SWITCH_PIN);
   }
}


