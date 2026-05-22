#include "usb_upload.h"

#include <string.h>

#define USB_RX_BUFFER_SIZE 4096

static char usb_rx_buffer[USB_RX_BUFFER_SIZE];
static int usb_rx_index;

volatile uint8_t usb_parsing_needed;
volatile uint8_t usb_accept_upload;
volatile uint8_t usb_rx_active;

void USB_Data_Receiver(uint8_t *buf, uint32_t len)
{
    if (len == 0U) {
        return;
    }

    /* Prevent buffer overflow but keep appending */
    if (usb_rx_index + (int)len >= USB_RX_BUFFER_SIZE) {
        len = USB_RX_BUFFER_SIZE - 1 - usb_rx_index;
    }

    memcpy(&usb_rx_buffer[usb_rx_index], buf, len);
    usb_rx_index += (int)len;
    usb_rx_buffer[usb_rx_index] = '\0';

    if (!usb_accept_upload) {
        return;
    }

    /* Check for START without wiping the real data attached to it */
    if (!usb_rx_active) {
        char *start_ptr = strstr(usb_rx_buffer, "START");
        if (start_ptr != NULL) {
            usb_rx_active = 1;
            
            char *data_start = start_ptr + 5; /* Skip "START" */
            if (*data_start == '\r') data_start++;
            if (*data_start == '\n') data_start++;

            int remaining = usb_rx_index - (int)(data_start - usb_rx_buffer);
            if (remaining > 0) {
                memmove(usb_rx_buffer, data_start, remaining);
                usb_rx_index = remaining;
            } else {
                usb_rx_index = 0;
            }
            usb_rx_buffer[usb_rx_index] = '\0';
        }
    }

    /* Check for END */
    if (usb_rx_active && strstr(usb_rx_buffer, "END") != NULL) {
        usb_parsing_needed = 1;
    }
}

uint8_t UsbUpload_IsAccepting(void)
{
    return usb_accept_upload;
}

void UsbUpload_SetAccepting(uint8_t accept)
{
    usb_accept_upload = accept;
}

uint8_t UsbUpload_IsRxActive(void)
{
    return usb_rx_active;
}

uint8_t UsbUpload_NeedsParsing(void)
{
    return usb_parsing_needed;
}

void UsbUpload_ClearParsingFlag(void)
{
    usb_parsing_needed = 0;
}

void UsbUpload_ResetBuffer(void)
{
    usb_rx_index = 0;
    memset(usb_rx_buffer, 0, sizeof(usb_rx_buffer));
    usb_parsing_needed = 0;
    usb_rx_active = 0;
}

char *UsbUpload_GetBuffer(void)
{
    return usb_rx_buffer;
}

uint32_t UsbUpload_GetLength(void)
{
    return (uint32_t)usb_rx_index;
}