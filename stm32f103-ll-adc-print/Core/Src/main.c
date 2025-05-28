
#include "main.h"
#include "usb_device.h"

#include "usbd_core.h"
#include "usbd_cdc.h"
#include "usbd_cdc_if.h"

#include "adc_read.h"

#include "SystemClock_Config.h"

extern USBD_DescriptorsTypeDef FS_Desc;
extern USBD_ClassTypeDef  USBD_CDC;
extern USBD_HandleTypeDef hUsbDeviceFS;

// void LL_mDelay(uint32_t Delay);
void SystemClock_Config(void);

// Функція налаштування піну для USB (для скидування USB)
void USB_GPIO_Init(void) {
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;

    GPIOA->CRH &= ~(GPIO_CRH_CNF11 | GPIO_CRH_CNF12);
    GPIOA->CRH |= (GPIO_CRH_CNF11_1 | GPIO_CRH_CNF12_1);

    RCC->APB1ENR |= RCC_APB1ENR_USBEN;
}

// Функція налаштування піну для USB (для скидування USB)
void USB_DEVICE_PinReset(void)
{
    // Увімкнення тактуючого сигналу для GPIOA
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;

    // Налаштування піну PA12 як вихід (Push-Pull, високошвидкісний)
    GPIOA->CRH &= ~(GPIO_CRH_MODE12 | GPIO_CRH_CNF12); // Очищаємо біти
    GPIOA->CRH |= (GPIO_CRH_MODE12_1 | GPIO_CRH_MODE12_0); // Output mode, 50 MHz
    GPIOA->CRH |= GPIO_CRH_CNF12_0; // General purpose output push-pull

    // Встановлення логічного рівня високого на PA12
    GPIOA->BSRR = GPIO_BSRR_BS12;

    // Невелика затримка (імітація)
    for (volatile int i = 0; i < 720000; i++); // ~10 мс (на 72 МГц)

    // Встановлення логічного рівня низького на PA12
    GPIOA->BSRR = GPIO_BSRR_BR12;
}

void USB_DEVICE_Init(void)
{
  USBD_Init(&hUsbDeviceFS, &FS_Desc, DEVICE_FS);
  USBD_RegisterClass(&hUsbDeviceFS, &USBD_CDC);
  USBD_CDC_RegisterInterface(&hUsbDeviceFS, &USBD_Interface_fops_FS);
  USBD_Start(&hUsbDeviceFS);
}

int main(void)
{

  /* USER CODE BEGIN 1 */

  SET_BIT(RCC->APB2ENR, RCC_APB2ENR_AFIOEN); // LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_AFIO);
  SET_BIT(RCC->APB1ENR, RCC_APB1ENR_PWREN); // LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);

  NVIC_SetPriorityGrouping(0x3U);
  MODIFY_REG(AFIO->MAPR, AFIO_MAPR_SWJ_CFG, AFIO_MAPR_SWJ_CFG_JTAGDISABLE); // LL_GPIO_AF_Remap_SWJ_NOJTAG();

  SystemClock_Config();

  USB_GPIO_Init();
  USB_DEVICE_PinReset();

  USB_DEVICE_Init();
  USBD_LL_SetSpeed(&hUsbDeviceFS, USBD_SPEED_FULL);
  USBD_LL_Reset(&hUsbDeviceFS);

  LL_mDelay(100);

  // Ініціалізуємо піни для PA0, PA1, PA2
  Init_ADC_Pin(GPIOA, 0); // Налаштувати PA0 як аналоговий вхід
  Init_ADC_Pin(GPIOA, 1); // Налаштувати PA0 як аналоговий вхід
  Init_ADC_Pin(GPIOA, 2); // Налаштувати PA0 як аналоговий вхід
  // Ініціалізація ADC
  Init_ADC(ADC1);

  // Читання значень з каналів PA0, PA1, PA2
  uint16_t value_PA0; // PA0
  uint16_t value_PA1; // PA1
  uint16_t value_PA2; // PA2

  uint8_t Temp_Buffer[APP_RX_DATA_SIZE] = { 0 };

  while (1)
  {
    // Читання значень з каналів PA0, PA1, PA2
    value_PA0 = Read_ADC(ADC1,0); // PA0
    value_PA1 = Read_ADC(ADC1,1); // PA1
    value_PA2 = Read_ADC(ADC1,2); // PA2

    // LL_mDelay(1);
    // printf("PA0 %04d\n\r", value_PA0); LL_mDelay(10);
    // printf("PA1 %04d\n\r", value_PA1); LL_mDelay(10);
    // printf("PA0 %04d$ PA1 %04d$\n\r\0", value_PA0,value_PA1);
    // printf("PA0 %04d$ PA1 %04d\0", value_PA0,value_PA1);
    // printf("\n\r"); for(uint32_t i=0;i<50;i++) __asm volatile ("nop");


    static char buff[20];
    snprintf(buff, 28, "PA0 %04d$ PA1 %04d$\n\0", value_PA0,value_PA1);
    CDC_Transmit_FS((uint8_t *)buff, strlen(buff));
    // printf("strlen %04d\n\0", strlen(buff)); // strlen 0020


    // uint8_t length = VCP_read_line(Temp_Buffer, sizeof(Temp_Buffer));
    // if(Temp_Buffer[0] != 0) {
    //   Temp_Buffer[length] = '\n';
    //
    //   if(length > 16)
    //   {
    //     printf("\n data %s \n", Temp_Buffer); LL_mDelay(2);
    //     // printf("length %d \n", length);
    //     LL_mDelay(2000);
    //   }
    //
    //   memset(Temp_Buffer, '\0', sizeof Temp_Buffer);
    //   for(uint16_t i =0;i<APP_RX_DATA_SIZE;i++) APP_RX_DATA[i] = '\0';
    //   length = 0;
    // }
  }
}


void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}

#ifdef  USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line) {}
#endif /* USE_FULL_ASSERT */

