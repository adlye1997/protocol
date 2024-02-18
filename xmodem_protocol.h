#ifndef __XMODEM_PROTOCOL_H
#define __XMODEM_PROTOCOL_H

#include "stm32f1xx_hal.h"

#define SOH 0x01    /* Start of Header */
#define STX 0x02    /* Start of Text */
#define EOT 0x04    /* End of Transmission */
#define ACK 0x06    /* Acknowledge */
#define NAK 0x15    /* Negative Acknowledge */
#define CAN 0x18    /* Cancel */
#define REQ 0x43    /* Request to send */

#define PacketDataSize 128
#define PacketSize (PacketDataSize+5)
#define XmodemTimeout 3000

typedef __packed struct
{
	uint8_t header;                // Header类型：SOH或者STX
	uint8_t packet_number;         // 数据包编号
	uint8_t packet_number_comp;    // 数据包编号的补码
	uint8_t data[PacketDataSize];  // 数据内容，128字节
	uint16_t crc;                  // CRC校验码
}XmodemPacketHeader;

typedef struct
{
	uint8_t port_status;
	uint16_t tick;
	void (*send_data)(uint8_t *data, uint16_t length);
}XmodemPort;

typedef enum
{
	READY = 0,
	RECEIVING = 1,
}XmodemPortStatus;

void XmodemPortInit(XmodemPort *port, void send_data(uint8_t *, uint16_t));
HAL_StatusTypeDef ExtractDataFromPacket(uint8_t *packet, uint8_t *data);
HAL_StatusTypeDef Unpack(XmodemPort *port, uint8_t *packet, uint16_t length, uint8_t *data, uint16_t *data_length);
void TramsmitPacket(uint8_t *data, uint16_t length, uint8_t *packet, uint8_t packet_number);
void XmodemTickTask(XmodemPort *port);

#endif
