# stm32f103-cdc-printf

``` c
diff -Naur stm32f103-usb-echo-4/Core/Src/main.c stm32f103-usb-echo-4b/Core/Src/main.c
--- stm32f103-usb-echo-4/Core/Src/main.c	2024-09-08 21:36:58.598000000 +0300
+++ stm32f103-usb-echo-4b/Core/Src/main.c	2024-09-08 21:32:36.589999522 +0300
@@ -23,7 +23,7 @@

 /* Private includes ----------------------------------------------------------*/
 /* USER CODE BEGIN Includes */
-
+#include "usb_scanf.h"
 /* USER CODE END Includes */

 /* Private typedef -----------------------------------------------------------*/
@@ -55,7 +55,15 @@

 /* Private user code ---------------------------------------------------------*/
 /* USER CODE BEGIN 0 */
-
+void USB_DEVICE_PinReset(void)
+{
+  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOA);
+  LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_12, LL_GPIO_MODE_OUTPUT);
+  LL_GPIO_SetPinSpeed(GPIOA, LL_GPIO_PIN_12, LL_GPIO_SPEED_FREQ_HIGH);
+  LL_GPIO_SetPinOutputType(GPIOA, LL_GPIO_PIN_12, LL_GPIO_OUTPUT_PUSHPULL);
+  LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_12);
+  LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_12);
+}
 /* USER CODE END 0 */

 /**
@@ -82,20 +90,34 @@
   SystemClock_Config();

   /* USER CODE BEGIN SysInit */
-
+  USB_DEVICE_PinReset();
   /* USER CODE END SysInit */

   /* Initialize all configured peripherals */
   MX_GPIO_Init();
   MX_USB_DEVICE_Init();
   /* USER CODE BEGIN 2 */
-
+  HAL_Delay(100);
+  uint8_t Temp_Buffer[APP_RX_DATA_SIZE] = { 0 };
   /* USER CODE END 2 */

   /* Infinite loop */
   /* USER CODE BEGIN WHILE */
   while (1)
   {
+    uint8_t length = VCP_read_line(Temp_Buffer, sizeof(Temp_Buffer));
+
+    if(Temp_Buffer[0] != 0) {
+      Temp_Buffer[length] = '\n';
+      printf("data %s \n", Temp_Buffer);
+      HAL_Delay(2);
+      printf("length %d \n", length);
+      HAL_Delay(2);
+
+      memset(Temp_Buffer, '\0', sizeof Temp_Buffer);
+      for(uint16_t i =0;i<APP_RX_DATA_SIZE;i++) APP_RX_DATA[i] = '\0';
+      length = 0;
+    }
     /* USER CODE END WHILE */

     /* USER CODE BEGIN 3 */
diff -Naur stm32f103-usb-echo-4/Core/usb_printf.c stm32f103-usb-echo-4b/Core/usb_printf.c
--- stm32f103-usb-echo-4/Core/usb_printf.c	1970-01-01 03:00:00.000000000 +0300
+++ stm32f103-usb-echo-4b/Core/usb_printf.c	2024-09-08 20:17:57.441640779 +0300
@@ -0,0 +1,9 @@
+// file usb_printf.c
+#include "usb_printf.h"
+
+int _write(int file, char *ptr, int len)
+{
+  CDC_Transmit_FS((uint8_t*) ptr, len);
+  return len;
+}
+
diff -Naur stm32f103-usb-echo-4/Core/usb_printf.h stm32f103-usb-echo-4b/Core/usb_printf.h
--- stm32f103-usb-echo-4/Core/usb_printf.h	1970-01-01 03:00:00.000000000 +0300
+++ stm32f103-usb-echo-4b/Core/usb_printf.h	2024-09-08 20:17:57.441640779 +0300
@@ -0,0 +1,18 @@
+#ifndef UART_SDC_PRINTF_H_
+#define UART_SDC_PRINTF_H_
+
+#ifdef __cplusplus
+extern "C" {
+#endif
+
+#include "usbd_cdc_if.h"
+#include "stdio.h"
+#include "stdarg.h"
+
+// int _write(int file, char *ptr, int len);
+
+#ifdef __cplusplus
+}
+#endif
+
+#endif /* UART_SDC_PRINTF_H_ */
diff -Naur stm32f103-usb-echo-4/Core/usb_scanf.c stm32f103-usb-echo-4b/Core/usb_scanf.c
--- stm32f103-usb-echo-4/Core/usb_scanf.c	1970-01-01 03:00:00.000000000 +0300
+++ stm32f103-usb-echo-4b/Core/usb_scanf.c	2024-09-08 20:17:57.441640779 +0300
@@ -0,0 +1,63 @@
+// file usb_scanf.c
+#include "usb_scanf.h"
+
+uint32_t head = 0;
+uint32_t tail = 0;
+uint8_t APP_RX_DATA[APP_RX_DATA_SIZE];
+
+// returns bytes read (could be zero)
+// would be easy to make it end early on a stop char (e.g., \r or \n)
+// повертає прочитані байти (може бути нулем)
+// було б легко зробити так, щоб це закінчилося раніше на символі зупинки (наприклад, \r або \n)
+uint8_t VCP_read(uint8_t* Buf, uint32_t Len)
+{
+  uint32_t count = 0;
+  /* Check inputs */
+  if ((Buf == NULL) || (Len == 0)) return 0;
+
+  while (Len--)
+  {
+    if (head == tail) return count;
+
+    count++;
+    *Buf++ = APP_RX_DATA[tail];
+
+    tail++;
+    if (tail >= APP_RX_DATA_SIZE) tail -= APP_RX_DATA_SIZE;
+  }
+
+  return count;
+}
+
+// повертає прочитані байти (може бути нулем), або раніше,
+// якщо зустрівся символі зупинки (\r або \n)
+uint8_t VCP_read_line(uint8_t* Buf, uint32_t Len)
+{
+  uint32_t count = 0;
+  /* Check inputs */
+  if ((Buf == NULL) || (Len == 0)) return 0;
+
+  memset(Buf, 0, Len);
+
+  while (Len--)
+  {
+    if (head == tail) return count;
+
+    if (APP_RX_DATA[tail] == '\r' || APP_RX_DATA[tail] == '\n')
+    {
+      *Buf = '\0';
+      tail++;
+      if (tail >= APP_RX_DATA_SIZE) tail -= APP_RX_DATA_SIZE;
+
+      break;
+    }
+
+    count++;
+    *Buf++ = APP_RX_DATA[tail];
+
+    tail++;
+    if (tail >= APP_RX_DATA_SIZE) tail -= APP_RX_DATA_SIZE;
+  }
+
+  return count;
+}
diff -Naur stm32f103-usb-echo-4/Core/usb_scanf.h stm32f103-usb-echo-4b/Core/usb_scanf.h
--- stm32f103-usb-echo-4/Core/usb_scanf.h	1970-01-01 03:00:00.000000000 +0300
+++ stm32f103-usb-echo-4b/Core/usb_scanf.h	2024-09-08 20:17:57.441640779 +0300
@@ -0,0 +1,28 @@
+#ifndef UART_SDC_SCANF_H_
+#define UART_SDC_SCANF_H_
+
+#ifdef __cplusplus
+extern "C" {
+#endif
+
+#include "usbd_cdc_if.h"
+#include "main.h"
+#include "stdio.h"
+#include "stdarg.h"
+
+extern uint32_t head;
+extern uint32_t tail;
+extern uint8_t APP_RX_DATA[];
+
+// returns bytes read (could be zero)
+// would be easy to make it end early on a stop char (e.g., \r or \n)
+// повертає прочитані байти (може бути нулем)
+// було б легко зробити так, щоб це закінчилося раніше на символі зупинки (наприклад, \r або \n)
+uint8_t VCP_read(uint8_t* Buf, uint32_t Len);
+uint8_t VCP_read_line(uint8_t* Buf, uint32_t Len);
+
+#ifdef __cplusplus
+}
+#endif
+
+#endif /* UART_SDC_SCANF_H_ */
diff -Naur stm32f103-usb-echo-4/USB_DEVICE/App/usbd_cdc_if.c stm32f103-usb-echo-4b/USB_DEVICE/App/usbd_cdc_if.c
--- stm32f103-usb-echo-4/USB_DEVICE/App/usbd_cdc_if.c	2024-09-08 21:36:57.650000000 +0300
+++ stm32f103-usb-echo-4b/USB_DEVICE/App/usbd_cdc_if.c	2024-09-08 21:34:57.154849101 +0300
@@ -261,6 +261,23 @@
   /* USER CODE BEGIN 6 */
   USBD_CDC_SetRxBuffer(&hUsbDeviceFS, &Buf[0]);
   USBD_CDC_ReceivePacket(&hUsbDeviceFS);
+
+  uint32_t len = *Len;
+
+  // add data to FIFO
+  while (len--)
+  {
+    if ((head+1) & (APP_RX_DATA_SIZE-1) == tail)
+    {
+      return USBD_FAIL; // overrun
+    }
+    else
+    {
+      APP_RX_DATA[head] = *Buf++;
+      if (++head >= APP_RX_DATA_SIZE) head -= APP_RX_DATA_SIZE;
+    }
+  }
+
   return (USBD_OK);
   /* USER CODE END 6 */
 }
diff -Naur stm32f103-usb-echo-4/USB_DEVICE/App/usbd_cdc_if.h stm32f103-usb-echo-4b/USB_DEVICE/App/usbd_cdc_if.h
--- stm32f103-usb-echo-4/USB_DEVICE/App/usbd_cdc_if.h	2024-09-08 21:36:57.524000000 +0300
+++ stm32f103-usb-echo-4b/USB_DEVICE/App/usbd_cdc_if.h	2024-09-08 21:35:09.177579886 +0300
@@ -30,7 +30,7 @@
 #include "usbd_cdc.h"

 /* USER CODE BEGIN INCLUDE */
-
+#include "usb_scanf.h"
 /* USER CODE END INCLUDE */

 /** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY

```
