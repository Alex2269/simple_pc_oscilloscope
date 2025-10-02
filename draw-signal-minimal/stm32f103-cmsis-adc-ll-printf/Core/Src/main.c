/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usb_device.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "adc_read.h"
#include "stm32f1xx_ll_adc.h"
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
// Функція налаштування піну для USB (для скидування USB)
void USB_DEVICE_PinReset(void)
{
  // Увімкнення тактуючого сигналу для порту A (для роботи з піном PA12)
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOA);

  // Налаштування піну PA12 як вихід
  LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_12, LL_GPIO_MODE_OUTPUT);
  LL_GPIO_SetPinSpeed(GPIOA, LL_GPIO_PIN_12, LL_GPIO_SPEED_FREQ_HIGH);
  LL_GPIO_SetPinOutputType(GPIOA, LL_GPIO_PIN_12, LL_GPIO_OUTPUT_PUSHPULL);

  // Встановлення логічного рівня високого на піні PA12 (активуємо скидання)
  LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_12);

  // Встановлення логічного рівня низького на піні PA12 (відключаємо скидання)
  LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_12);
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  USB_DEVICE_PinReset();
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  // MX_GPIO_Init();
  MX_USB_DEVICE_Init();
  /* USER CODE BEGIN 2 */

  LL_mDelay(100);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  // Ініціалізуємо піни для PA0, PA1, PA2
  // Init_ADC_Pin(GPIOA, LL_GPIO_PIN_0);  // PA0
  // Init_ADC_Pin(GPIOA, LL_GPIO_PIN_1);  // PA1
  // Init_ADC_Pin(GPIOA, LL_GPIO_PIN_2);  // PA2
  Init_ADC_Pin(GPIOA, 0); // Налаштувати PA0 як аналоговий вхід
  Init_ADC_Pin(GPIOA, 1); // Налаштувати PA0 як аналоговий вхід
  Init_ADC_Pin(GPIOA, 2); // Налаштувати PA0 як аналоговий вхід
  // Ініціалізація ADC
  Init_ADC(ADC1);

  // Читання значень з каналів PA0, PA1, PA2
  uint16_t value_PA0; // PA0
  uint16_t value_PA1; // PA1
  uint16_t value_PA2; // PA2
  while (1)
  {
    // Читання значень з каналів PA0, PA1, PA2
    value_PA0 = Read_ADC(ADC1,0); // PA0
    value_PA1 = Read_ADC(ADC1,1); // PA1
    value_PA2 = Read_ADC(ADC1,2); // PA2

    printf("PA0 %d \t", value_PA0); // LL_mDelay(1);
    printf("PA1 %d \n", value_PA1); // LL_mDelay(1);

    // printf("PA0 %d \t", value_PA0); // LL_mDelay(1);
    // printf("PA1 %d \t", value_PA1); // LL_mDelay(1);
    // printf("PA2 %d \n", value_PA2); LL_mDelay(250);

    /* USER CODE END WHILE */

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
  LL_FLASH_SetLatency(LL_FLASH_LATENCY_2);
  while(LL_FLASH_GetLatency()!= LL_FLASH_LATENCY_2)
  {
  }
  LL_RCC_HSE_Enable();

   /* Wait till HSE is ready */
  while(LL_RCC_HSE_IsReady() != 1)
  {

  }
  LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSE_DIV_1, LL_RCC_PLL_MUL_9);
  LL_RCC_PLL_Enable();

   /* Wait till PLL is ready */
  while(LL_RCC_PLL_IsReady() != 1)
  {

  }
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_2);
  LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);

   /* Wait till System clock is ready */
  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL)
  {

  }
  LL_SetSystemCoreClock(72000000);
  LL_Init1msTick(72000000);

   /* Update the time base */
  if (HAL_InitTick (TICK_INT_PRIORITY) != HAL_OK)
  {
    Error_Handler();
  }
  LL_RCC_SetUSBClockSource(LL_RCC_USB_CLKSOURCE_PLL_DIV_1_5);
  LL_RCC_SetADCClockSource(LL_RCC_ADC_CLKSRC_PCLK2_DIV_6);
}

/* USER CODE BEGIN 4 */
/*
// func from stm32f1xx_ll_utils.c
uint32_t SystemCoreClock;
void LL_SetSystemCoreClock(uint32_t HCLKFrequency)
{
  SystemCoreClock = HCLKFrequency;
}

// func from stm32f1xx_ll_utils.c
void LL_Init1msTick(uint32_t HCLKFrequency)
{
  LL_InitTick(HCLKFrequency, 1000U);
}*/
/*
void SystemInit(void)
{
  #if defined(USER_VECT_TAB_ADDRESS)
  SCB->VTOR = 0x08000000UL;
  #endif
}*/

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
