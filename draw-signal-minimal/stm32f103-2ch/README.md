# stm32f103-cdc-printf

``` c
// file usb_printf.c
#include "usb_printf.h"

int _write(int file, char *ptr, int len)
{
  CDC_Transmit_FS((uint8_t*) ptr, len);
  return len;
}
```
