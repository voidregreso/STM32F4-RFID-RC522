/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "spi.h"
#include "usart.h"
#include "gpio.h"
#include "key.h"
#include "MFRC522.h"
#include <string.h>

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
	uint8_t sta = 0, codigo = 0;
	uint16_t i, j;
	uint8_t cardstr[17], str[17];
	uint8_t sectorX_KeyA[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
	uint8_t escribir[]="DatosDesdeMexico";
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_SPI1_Init();
  /* USER CODE BEGIN 2 */
	
  HAL_GPIO_WritePin(RC522_Rst_GPIO_Port, RC522_Rst_Pin, GPIO_PIN_SET);
  HAL_Delay(100);
  MFRC522_Init();
	sta = Read_MFRC522(VersionReg);
	printf("MFRC522 version: %x\r\n", sta);
	HAL_Delay(1000);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1) {
    /* USER CODE END WHILE */
		memset(cardstr, 0, sizeof(cardstr));
		sta = MFRC522_Request(PICC_REQIDL, cardstr);
		if(sta == MI_OK) { 
			HAL_GPIO_TogglePin(GPIOF, GPIO_PIN_9);
			printf("Card: %x,%x,%x\r\n", cardstr[0], cardstr[1], cardstr[2]);
			HAL_Delay(1);
			sta = MFRC522_Anticoll(cardstr);
			if(sta == MI_OK) {
				printf("UID: %x %x %x %x\r\n", cardstr[0], cardstr[1], cardstr[2], cardstr[3]);
				HAL_Delay(1);
				sta = MFRC522_SelectTag(cardstr);
				if(sta > 0) {
					codigo = KeyScan(1);
					switch(codigo) {
						case 1: //key0, read
							HAL_GPIO_TogglePin(GPIOF, GPIO_PIN_10);
							HAL_Delay(1);
							sta = MFRC522_Auth(0x60, 0, sectorX_KeyA, cardstr);	//sector 0
							printf("Authentication completed with status code %x\r\n", sta);
							for(i = 0; i < 16; i++) {
								for(j = i*4; j < (i+1)*4; j++) {
									sta = MFRC522_Read(j, str);
									printf("Cluster %d, Status = %d, ReadString=%s\r\n", j, sta, str);
								}
							}
							MFRC522_Halt();
							HAL_Delay(1);
							break;
						case 2: //key1, write
							HAL_GPIO_TogglePin(GPIOF, GPIO_PIN_9);
							HAL_GPIO_TogglePin(GPIOF, GPIO_PIN_10);
							HAL_Delay(1);
							sta = MFRC522_Auth(0x60,4,sectorX_KeyA, cardstr);	//sector 1
							if(sta == MI_OK) {
								sta = MFRC522_Write(4, escribir);
								printf("Write completed with status code %x\r\n", sta);
							}
							MFRC522_Halt();
							HAL_Delay(1);
							break;
						default:
							printf("Invalid key pressed down!\r\n");
							break;
					}
				}
			}
		}
    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
