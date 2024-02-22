#include "string.h"
#include "crc16.h"
#include "bootloader.h"

#include "mydebug.h"

/*
test data
01 00 ff 00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f 10 11 12 13 14 15 16 17 18 19 1a 1b 1c 1d 1e 1f 20 21 22 23 24 25 26 27 28 29 2a 2b 2c 2d 2e 2f 30 31 32 33 34 35 36 37 38 39 3a 3b 3c 3d 3e 3f 40 41 42 43 44 45 46 47 48 49 4a 4b 4c 4d 4e 4f 50 51 52 53 54 55 56 57 58 59 5a 5b 5c 5d 5e 5f 60 61 62 63 64 65 66 67 68 69 6a 6b 6c 6d 6e 6f 70 71 72 73 74 75 76 77 78 79 7a 7b 7c 7d 7e 7f 2d 64
*/

/***
 * example
Xmodem xmodem_boot;
extern Boot boot;

static void BootXmodemTransmit(uint8_t *txbuf, uint16_t len) {
}

static void BootXmodemReceiveCallback(uint8_t *rxbuf, uint16_t len) {
	uint8_t data[300];
	uint16_t data_len;
	XmodemUpdate(&xmodem_boot);
	if(Unpack(&xmodem_boot, rxbuf, len, data, &data_len) == HAL_OK) {
		boot.ReceiveCallback(data, data_len);
	}
}

static void BootXmodemBegin(void) {
	SendReq(&xmodem_boot);
	xmodem_boot.status = XMODEM_RECEIVING;
}
*/

// extern Port port_uart1;
void XmodemSetup(void) {
	// XmodemInit(&xmodem_boot, 10000, &port_uart1, BootXmodemTransmit, \
	// 			BootXmodemReceiveCallback, BootXmodemBegin);
}

void XmodemLoop(void) {
	// XmodemPolling(&xmodem_boot);
}

HAL_StatusTypeDef Unpack(Xmodem *xmodem, uint8_t *packet, uint16_t length, \
			uint8_t *data, uint16_t *data_length)
{
	if(length == XMODEM_PACKET_SIZE && xmodem->status == XMODEM_RECEIVING) {
		if(ExtractDataFromPacket(packet, data) == HAL_OK) {
			SendAck(xmodem);
			*data_length = XMODEM_PACKET_DATA_SIZE;
			return HAL_OK;
		}
		else {
			SendNak(xmodem);
		}
	}
	else if(length == 1) {
		switch(packet[0]) {
			case XMODEM_EOT:
				if(xmodem->status == XMODEM_RECEIVING) {
					xmodem->status = XMODEM_END;
					SendAck(xmodem);
				}
				break;
			default:
				break;
		}
	}
	return HAL_ERROR;
}

/**
 * @brief Xmodem结构体初始化
 *
 */
static void XmodemInit(Xmodem *xmodem, uint16_t timeout, Port *port, \
			void Transmit(uint8_t*, uint16_t), \
			void ReceiveCallback(uint8_t*, uint16_t), void Begin(void)) {
	xmodem->status = XMODEM_READY;
	xmodem->tick_update = HAL_GetTick();
	xmodem->tick_timeout = timeout;
	xmodem->packet_number = 0;
	xmodem->port = port;
	xmodem->Transmit = Transmit;
	xmodem->ReceiveCallback = ReceiveCallback;
	xmodem->Begin = Begin;
}

static void XmodemUpdate(Xmodem *xmodem) {
	xmodem->tick_update = HAL_GetTick();
}

static void XmodemPolling(Xmodem *xmodem) {
	if(xmodem->status == XMODEM_RECEIVING) {
		if(HAL_GetTick() - xmodem->tick_update >= xmodem->tick_timeout) {
			xmodem->status = XMODEM_TIMEOUT;
		}
	}
}

XmodemHeader received_packet;
XmodemHeader tramsmit_packet;

/**
 * @brief 从packet提取data
 *
 * @param packet
 * @param data
 * @return HAL_StatusTypeDef
 */
static HAL_StatusTypeDef ExtractDataFromPacket(uint8_t *packet, uint8_t *data) {
	memcpy(&received_packet, packet, sizeof(received_packet));

	if(received_packet.header != XMODEM_SOH)
	{
		return HAL_ERROR;
	}

	if(received_packet.packet_number + received_packet.packet_number_comp != \
				0xFF)
	{
		return HAL_ERROR;
	}

	if(!VerifyCRC16(packet, XMODEM_PACKET_SIZE))
	{
		return HAL_ERROR;
	}

	memcpy(data, received_packet.data, sizeof(received_packet.data));
	return HAL_OK;
}

/**
 * @brief 将data整合成packet
 *
 * @param data
 * @param length
 * @param packet
 * @param packet_number
 */
static void ConsolidateDataIntoPacket(uint8_t *data, uint16_t length, \
			uint8_t *packet, uint8_t packet_number) {
	tramsmit_packet.header = XMODEM_SOH;
	tramsmit_packet.packet_number = packet_number;
	tramsmit_packet.packet_number_comp = ~tramsmit_packet.packet_number;
	memset(&tramsmit_packet.data, 0, sizeof(tramsmit_packet.data));
	memcpy(&tramsmit_packet.data, data, length);
	memcpy(packet, &tramsmit_packet, sizeof(tramsmit_packet));
	AppendCRC16(packet, XMODEM_PACKET_SIZE);
}

static void SendAck(Xmodem *xmodem) {
	uint8_t txbuf = XMODEM_ACK;
	xmodem->port->Transmit(&txbuf, 1);
}

static void SendNak(Xmodem *xmodem) {
	uint8_t txbuf = XMODEM_NAK;
	xmodem->port->Transmit(&txbuf, 1);
}

static void SendCan(Xmodem *xmodem) {
	uint8_t txbuf = XMODEM_CAN;
	xmodem->port->Transmit(&txbuf, 1);
}

static void SendReq(Xmodem *xmodem) {
	uint8_t txbuf = XMODEM_REQ;
	xmodem->port->Transmit(&txbuf, 1);
}

