#ifndef _DEBUGPINS__H_
#define _DEBUGPINS__H_

/* test pin on C port */
#define TEST_PIN GPIO_Pin_13

void initTestPin(void);
void setTestPin(void);
void clearTestPin(void);
void toggleTestPin(void);

#endif