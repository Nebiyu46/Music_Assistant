#ifndef USB_UPLOAD_H
#define USB_UPLOAD_H

#include <stdint.h>

void USB_Data_Receiver(uint8_t *buf, uint32_t len);

uint8_t UsbUpload_IsAccepting(void);
void    UsbUpload_SetAccepting(uint8_t accept);
uint8_t UsbUpload_IsRxActive(void);
uint8_t UsbUpload_NeedsParsing(void);
void    UsbUpload_ClearParsingFlag(void);
void    UsbUpload_ResetBuffer(void);
char *UsbUpload_GetBuffer(void);

#endif /* USB_UPLOAD_H */
