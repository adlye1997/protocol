#ifndef __CRC16_H
#define __CRC16_H

#include "stm32f1xx_hal.h"
#include "stdbool.h"

uint16_t GetCRC16(uint8_t *pchMessage, uint32_t dwLength, uint16_t wCRC);
void AppendCRC16(uint8_t * pchMessage, uint32_t dwLength);
bool VerifyCRC16(uint8_t *pchMessage, uint32_t dwLength);

#endif




