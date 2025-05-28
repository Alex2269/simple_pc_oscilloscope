#ifndef UART_SDC_SCANF_H_
#define UART_SDC_SCANF_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "usbd_cdc_if.h"
#include "main.h"
#include "stdio.h"
#include "stdarg.h"

extern uint32_t head;
extern uint32_t tail;
extern uint8_t APP_RX_DATA[];

// returns bytes read (could be zero)
// would be easy to make it end early on a stop char (e.g., \r or \n)
// повертає прочитані байти (може бути нулем)
// було б легко зробити так, щоб це закінчилося раніше на символі зупинки (наприклад, \r або \n)
uint8_t VCP_read(uint8_t* Buf, uint32_t Len);
uint8_t VCP_read_line(uint8_t* Buf, uint32_t Len);

#ifdef __cplusplus
}
#endif

#endif /* UART_SDC_SCANF_H_ */
