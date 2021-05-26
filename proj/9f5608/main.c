#include "stm32f4xx.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_usart.h"
#include "../usart/uart.h"
#include "../common/dbg_pin.h"


#define R_B_PIN GPIO_Pin_0
#define CLE_PIN GPIO_Pin_1
#define ALE_PIN GPIO_Pin_2
#define _CE_PIN GPIO_Pin_6
#define _RE_PIN GPIO_Pin_4
#define _WE_PIN GPIO_Pin_5

#define SET_CLE()	GPIO_SetBits(GPIOC, CLE_PIN)
#define RESET_CLE()	GPIO_ResetBits(GPIOC, CLE_PIN)

#define SET_ALE() 	GPIO_SetBits(GPIOC, ALE_PIN)
#define RESET_ALE()	GPIO_ResetBits(GPIOC, ALE_PIN)

#define SET_CE() 	GPIO_SetBits(GPIOC, _CE_PIN)
#define RESET_CE()	GPIO_ResetBits(GPIOC, _CE_PIN)

#define SET_RE() 	GPIO_SetBits(GPIOC, _RE_PIN)
#define RESET_RE()	GPIO_ResetBits(GPIOC, _RE_PIN)

#define SET_WE() 	GPIO_SetBits(GPIOC, _WE_PIN)
#define RESET_WE()	GPIO_ResetBits(GPIOC, _WE_PIN)

#define R_B_PIN_STATE 	GPIO_ReadOutputDataBit(GPIOC, R_B_PIN)

#define IO_PINS 	(GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_6 | GPIO_Pin_7)

#define DELAY_10() \
				do { \
					asm("nop"); \
					asm("nop"); \
					} while (0)
#define DELAY_20() \
				do { \
					DELAY_10(); \
					DELAY_10(); \
					} while (0)

#define DELAY_30() \
				do { \
					DELAY_20(); \
					asm("nop"); \
					} while (0)

#define DELAY_40() \
				do { \
					DELAY_20(); \
					DELAY_20(); \
					} while (0)
#define DELAY_60() \
				do { \
					DELAY_40(); \
					DELAY_40(); \
					} while (0)


volatile uint32_t msTicks = 0;

void delay_ms(uint32_t ms)
{
	uint32_t ticks = msTicks + ms;
	
	while(ticks > msTicks);
	while(ticks < msTicks);

}

void SysTick_Handler()
{	
	msTicks++;
}

void initPins()
{

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStruct;
	
	GPIO_StructInit(&GPIO_InitStruct);	
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;	//IO pins
    GPIO_InitStruct.GPIO_Pin = IO_PINS;
    GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	//R/_B pin
	GPIO_InitStruct.GPIO_Pin = R_B_PIN;
	GPIO_Init(GPIOC, &GPIO_InitStruct);
								//CLE, ALE, _CE, _RE, _WE,
	GPIO_InitStruct.GPIO_Pin = CLE_PIN | ALE_PIN | _CE_PIN | _RE_PIN | _WE_PIN;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_Init(GPIOC, &GPIO_InitStruct);
}


void setIO_Out()
{
	GPIOB->MODER = 0x5555;
	GPIOB->OTYPER  = 0x00;
}

void setIO_IN()
{
	GPIOB->MODER = 0x00;
	GPIOB->OTYPER = 0xFF;
}

void setPinsDefault()
{
	RESET_CLE();
	RESET_ALE();
	SET_RE();
	SET_CE();
	SET_WE();
}

uint8_t readID()
{
	SET_CLE();
	RESET_CE();
	RESET_WE(); //30ns to set
	setIO_Out();
	GPIO_Write(GPIOB, 0x90); DELAY_20(); //20ns to WE
	SET_WE();	//keep data for 10ns after
	DELAY_10(); RESET_CLE(); SET_ALE();//10ns after WE to CLE and ALE
	
	GPIO_Write(GPIOB, 0x00);
	RESET_WE(); DELAY_10();
	SET_WE();
	RESET_ALE(); DELAY_10();

	setIO_IN();
	RESET_RE(); DELAY_40(); //35ns to data
	//Maker code here
	SET_RE(); DELAY_30(); //25 pulse width
	RESET_RE(); DELAY_40(); //35ns to data
	
	uint8_t status = GPIO_ReadInputData(GPIOB) & 0xFF;

	setPinsDefault();
	return status;
}

void readPage(uint8_t addr[3], uint8_t *data, uint16_t size)
{
	SET_CLE();
	RESET_CE();
	setIO_Out();

	GPIO_Write(GPIOB, 0x00);
	RESET_WE();
	//
	SET_WE();
	RESET_CLE();
	SET_ALE();
	
	for(int i = 0; i < 3; ++i)
	{
		GPIO_Write(GPIOB, addr[i]);
		RESET_WE();
		DELAY_10(); DELAY_10();
		SET_WE();
	}
	RESET_ALE();
	while( R_B_PIN_STATE ); DELAY_20(); //20ns to RE
	
	setIO_IN();
	for(int i = 0; i < size; ++i)
	{
		RESET_RE(); DELAY_40(); //35ns to data
		data[i] = GPIO_ReadInputData(GPIOB) & 0xFF;
		SET_RE(); DELAY_20(); //15ns to RE
	}
	setPinsDefault();
}

int main(void) {
  
	SystemInit();
	SysTick_Config(SystemCoreClock / 1000);
	InitTestPin();

	USART2_Init();
	
	initPins();
	setPinsDefault();
	
	uint8_t status = readID();
	USART2_Send_Byte(status);
	uint8_t addr[] = {0,0,0};
	//readPage(&addr[0], &addr[0], 3);
	//USART2_SendNoneBlocking(&addr[0], 3);
	
	SetTestPin();
	for(;;){
		
	}
}