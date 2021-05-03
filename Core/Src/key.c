#include <key.h>

#define KEY0        HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_4)  //KEY0
#define KEY1        HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_3)  //KEY1
#define KEY2        HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_2) 	//KEY2
#define WK_UP       HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_0)  //WKUP

uint8_t KeyScan(uint8_t lpress) {
	static uint8_t key_up = 1;
	if(lpress) key_up = 1;
	if(key_up && (!KEY0 || !KEY1 || !KEY2 || WK_UP)) {
		HAL_Delay(10);
		key_up = 0;
		if(!KEY0) return 1; // key0 pressed down
		else if(!KEY1) return 2;
		else if(!KEY2) return 3;
		else if(WK_UP) return 4;
	} else if(KEY0 && KEY1 && KEY2 && !WK_UP) key_up = 1;
	return 0; // No button pressed down!
}