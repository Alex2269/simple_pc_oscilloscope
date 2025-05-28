// file usb_scanf.c
#include "usb_scanf.h"

uint32_t head = 0;
uint32_t tail = 0;
uint8_t APP_RX_DATA[APP_RX_DATA_SIZE];

// returns bytes read (could be zero)
// would be easy to make it end early on a stop char (e.g., \r or \n)
// повертає прочитані байти (може бути нулем)
// було б легко зробити так, щоб це закінчилося раніше на символі зупинки (наприклад, \r або \n)
uint8_t VCP_read(uint8_t* Buf, uint32_t Len)
{
  uint32_t count = 0;
  /* Check inputs */
  if ((Buf == NULL) || (Len == 0)) return 0;

  while (Len--)
  {
    if (head == tail) return count;

    count++;
    *Buf++ = APP_RX_DATA[tail];

    tail++;
    if (tail >= APP_RX_DATA_SIZE) tail -= APP_RX_DATA_SIZE;
  }

  return count;
}

// повертає прочитані байти (може бути нулем), або раніше,
// якщо зустрівся символі зупинки (\r або \n)
uint8_t VCP_read_line(uint8_t* Buf, uint32_t Len)
{
  uint32_t count = 0;
  /* Check inputs */
  if ((Buf == NULL) || (Len == 0)) return 0;

  memset(Buf, 0, Len);

  while (Len--)
  {
    if (head == tail) return count;

    if (APP_RX_DATA[tail] == '\r' || APP_RX_DATA[tail] == '\n')
    {
      *Buf = '\0';
      tail++;
      if (tail >= APP_RX_DATA_SIZE) tail -= APP_RX_DATA_SIZE;

      break;
    }

    count++;
    *Buf++ = APP_RX_DATA[tail];

    tail++;
    if (tail >= APP_RX_DATA_SIZE) tail -= APP_RX_DATA_SIZE;
  }

  return count;
}
