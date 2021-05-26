#ifndef _DBG_PIN_H_
#define _DBG_PIN_H_

#define TEST_PORT GPIOD

#define LED1 GPIO_Pin_15
#define LED2 GPIO_Pin_14
#define LED3 GPIO_Pin_13
#define LED4 GPIO_Pin_12

#define SetTestPin1() SetTestLed(LED1)
#define SetTestPin2() SetTestLed(LED2)
#define SetTestPin3() SetTestLed(LED3)
#define SetTestPin4() SetTestLed(LED4)

#define ClearTestPin1() ClearTestLed(LED1)
#define ClearTestPin2() ClearTestLed(LED2)
#define ClearTestPin3() ClearTestLed(LED3)
#define ClearTestPin4() ClearTestLed(LED4)

#define ToggleTestPin1() ToggleTestLed(LED1)
#define ToggleTestPin2() ToggleTestLed(LED2)
#define ToggleTestPin3() ToggleTestLed(LED3)
#define ToggleTestPin4() ToggleTestLed(LED4)

#define SetTestPin() SetTestPin1()
#define ClearTestPin() ClearTestPin1()
#define ToggleTestPin() ToggleTestPin1()

void InitTestPin(void);
void SetTestLed(uint16_t LEDx);
void ClearTestLed(uint16_t LEDx);
void ToggleTestLed(uint16_t LEDx);

#endif
