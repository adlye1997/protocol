#include "xmodem_protocol.h"
#include "string.h"
#include "crc16.h"
#include "usart.h"

/*
test data
01 00 ff 00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f 10 11 12 13 14 15 16 17 18 19 1a 1b 1c 1d 1e 1f 20 21 22 23 24 25 26 27 28 29 2a 2b 2c 2d 2e 2f 30 31 32 33 34 35 36 37 38 39 3a 3b 3c 3d 3e 3f 40 41 42 43 44 45 46 47 48 49 4a 4b 4c 4d 4e 4f 50 51 52 53 54 55 56 57 58 59 5a 5b 5c 5d 5e 5f 60 61 62 63 64 65 66 67 68 69 6a 6b 6c 6d 6e 6f 70 71 72 73 74 75 76 77 78 79 7a 7b 7c 7d 7e 7f 2d 64
*/

XmodemPacketHeader received_packet;
XmodemPacketHeader tramsmit_packet;

void XmodemPortInit(XmodemPort *port, void send_data(uint8_t *, uint16_t))
{
	port->port_status = READY;
	port->send_data = send_data;
}

HAL_StatusTypeDef ExtractDataFromPacket(uint8_t *packet, uint8_t *data)
{
	memcpy(&received_packet, packet, sizeof(received_packet));

	if(received_packet.header != SOH)
	{
		return HAL_ERROR;
	}

	if(received_packet.packet_number + received_packet.packet_number_comp != 0xFF)
	{
		return HAL_ERROR;
	}

	if(!VerifyCRC16(packet, PacketSize))
	{
		return HAL_ERROR;
	}

	memcpy(data, received_packet.data, sizeof(received_packet.data));
	return HAL_OK;
}

HAL_StatusTypeDef Unpack(XmodemPort *port, uint8_t *packet, uint16_t length, uint8_t *data, uint16_t *data_length)
{
	if(length < 3)
	{
		switch(packet[0])
		{
			case REQ:
				if(port->port_status == READY)
				{
					port->port_status = RECEIVING;
					port->tick = HAL_GetTick();
					if(port->send_data == NULL)
					{
						return HAL_ERROR;
					}
					uint8_t tx = ACK;
					port->send_data(&tx, 1);
				}
				return HAL_ERROR;
			case EOT:
				if(port->port_status == RECEIVING)
				{
					port->port_status = READY;
					uint8_t tx = ACK;
					port->send_data(&tx, 1);
				}
				return HAL_ERROR;
			default:
				return HAL_ERROR;
		}
	}
	else if(length == PacketSize && port->port_status == RECEIVING)
	{
		if(ExtractDataFromPacket(packet, data) == HAL_OK)
		{
			uint8_t tx = ACK;
			port->send_data(&tx, 1);
		}
		else
		{
			uint8_t tx = NAK;
			port->send_data(&tx, 1);
		}
	}
	else
	{
		return HAL_ERROR;
	}
	*data_length = PacketDataSize;
	return HAL_OK;
}

void XmodemTickTask(XmodemPort *port)
{
	if(HAL_GetTick() - port->tick >= XmodemTimeout)
	{
		port->port_status = READY;
	}
}

void TramsmitPacket(uint8_t *data, uint16_t length, uint8_t *packet, uint8_t packet_number)
{
	tramsmit_packet.header = SOH;
	tramsmit_packet.packet_number = packet_number;
	tramsmit_packet.packet_number_comp = ~tramsmit_packet.packet_number;
	memset(&tramsmit_packet.data, 0, sizeof(tramsmit_packet.data));
	memcpy(&tramsmit_packet.data, data, length);
	memcpy(packet, &tramsmit_packet, sizeof(tramsmit_packet));
	AppendCRC16(packet, 133);
}
