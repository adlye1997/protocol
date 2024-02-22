/**
 * @file protocol_xmodem.h
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2024-02-22
 * @bug 结构体不需要发送数据的接口，缺少取消接收的接口
 *
 */
#define __PROTOCOL_XMODEM_H

#include "stm32f1xx_hal.h"
#include "mid_port.h"

#define XMODEM_SOH 0x01    /* Start of Header */
#define XMODEM_STX 0x02    /* Start of Text */
#define XMODEM_EOT 0x04    /* End of Transmission */
#define XMODEM_ACK 0x06    /* Acknowledge */
#define XMODEM_NAK 0x15    /* Negative Acknowledge */
#define XMODEM_CAN 0x18    /* Cancel */
#define XMODEM_REQ 0x43    /* Request to send */

#define XMODEM_PACKET_DATA_SIZE 128
#define XMODEM_PACKET_SIZE (XMODEM_PACKET_DATA_SIZE + 5)

typedef __packed struct {
	uint8_t header;                         // Header类型：SOH或者STX
	uint8_t packet_number;                  // 数据包编号
	uint8_t packet_number_comp;             // 数据包编号的补码
	uint8_t data[XMODEM_PACKET_DATA_SIZE];  // 数据内容，128字节
	uint16_t crc;                           // CRC校验码
}XmodemHeader;

typedef struct {
	uint8_t status;
	uint16_t tick_update;
	uint16_t tick_timeout;
	uint8_t packet_number;
	Port *port;
	void (*Transmit)(uint8_t *a, uint16_t b);
	void (*ReceiveCallback)(uint8_t *a, uint16_t b);
	void (*Begin)(void);
}Xmodem;

typedef enum {
	XMODEM_READY,
	XMODEM_RECEIVING,
	XMODEM_TIMEOUT,
	XMODEM_ERROR,
	XMODEM_END
}XmodemStatus;

void XmodemSetup(void);
void XmodemLoop(void);
HAL_StatusTypeDef Unpack(Xmodem *xmodem, uint8_t *packet, uint16_t length, \
			uint8_t *data, uint16_t *data_length);

static void XmodemInit(Xmodem *xmodem, uint16_t timeout, Port *port, \
			void Transmit(uint8_t*, uint16_t), \
			void ReceiveCallback(uint8_t*, uint16_t), void Begin(void));
static void XmodemUpdate(Xmodem *xmodem);
static void XmodemPolling(Xmodem *xmodem);
static HAL_StatusTypeDef ExtractDataFromPacket(uint8_t *packet, uint8_t *data);
static void ConsolidateDataIntoPacket(uint8_t *data, uint16_t length, \
			uint8_t *packet, uint8_t packet_number);

static void SendAck(Xmodem *xmodem);
static void SendNak(Xmodem *xmodem);
static void SendCan(Xmodem *xmodem);
static void SendReq(Xmodem *xmodem);

#endif // !__PROTOCOL_XMODEM_H
