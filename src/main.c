#include <stm32f10x_map.h>
int main(void)
{
	for(;;) {
		u32 a = GPIOA->CRH;
		a = (a & ~0xf0) | 0x90;
		GPIOA->CRH = a;
	}
}
