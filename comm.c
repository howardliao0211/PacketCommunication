#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "comm.h"

static void COMM_Reset(struct COMM *self)
{
    self->header_found      = false;
    self->status            = COMM_STATUS_FAIL;
    self->rx_buf_rear       = 0;
    self->tx_payload_ptr    = &self->tx_buf[COMM_PAYLOAD_OFFSET];
}

static uint8_t COMM_ChecksumCal(uint8_t *buf, uint16_t size)
{
    uint8_t checksum = buf[0];

    for(int i = 1; i < size; i++)
    {
        checksum += buf[i];
    }

    return checksum;
}

static COMM_STATUS COMM_FindPacket(struct COMM *self)
{
    if( self->header_found )
    {
        return self->status;
    }

    if( self->rx_buf_rear < COMM_MIN_SIZE )
    {
        return COMM_STATUS_LENGTH_ERROR;
    }

    for(int i = 0; i < self->rx_buf_rear; i++)
    {
        if( (self->rx_buf[i + 0] != COMM_HEADER[0]) ||
            (self->rx_buf[i + 1] != COMM_HEADER[1]) ||
            (self->rx_buf[i + 2] != COMM_HEADER[2]) ||
            (self->rx_buf[i + 3] != COMM_HEADER[3]) ||
            (self->rx_buf[i + COMM_DIR_OFFSET] != COMM_DIR_WRITE)
        )
        {
            continue;
        }

        self->header_found      = true;
        self->header_idx        = i;
        self->cmd               = self->rx_buf[i + COMM_COMMAND_OFFSET];
        self->rx_payload_len    = ((uint16_t) self->rx_buf[i + COMM_PAYLOAD_LEN_OFFSET + 0] << 8)
                                + ((uint16_t) self->rx_buf[i + COMM_PAYLOAD_LEN_OFFSET + 1]);
        self->rx_payload_ptr    = self->rx_buf + i + COMM_PAYLOAD_OFFSET;

        return COMM_STATUS_SUCCESS;
    }

    return COMM_STATUS_MISSING_HEADER;
}

static COMM_STATUS COMM_ChecksumCheck(struct COMM *self)
{
    uint8_t checksum;

    if( !self->header_found )
    {
        return COMM_STATUS_MISSING_HEADER;
    }

    checksum = COMM_ChecksumCal(&self->rx_buf[self->header_idx], COMM_MIN_SIZE + self->rx_payload_len);

    return (checksum) ? COMM_STATUS_CHECKSUM_FAIL : COMM_STATUS_SUCCESS;
}

static void COMM_SendResponse(struct COMM *self)
{
    uint8_t checksum;
    uint16_t checksum_offset = COMM_PAYLOAD_OFFSET + self->tx_payload_len;

    memcpy(self->tx_buf, COMM_HEADER, COMM_HEADER_SIZE);
    self->tx_buf[COMM_DIR_OFFSET]               = COMM_DIR_READ;
    self->tx_buf[COMM_COMMAND_OFFSET]           = self->cmd;
    self->tx_buf[COMM_PAYLOAD_LEN_OFFSET]       = (uint8_t) self->tx_payload_len >> 8;
    self->tx_buf[COMM_PAYLOAD_LEN_OFFSET + 1]   = (uint8_t) self->tx_payload_len & 0xFF;

    checksum = COMM_ChecksumCal(self->tx_buf, checksum_offset);
    self->tx_buf[checksum_offset] = ~(checksum) + 1;
    self->tx_buf_rear = COMM_MIN_SIZE + self->tx_payload_len;

    COMM_Send(self);
}

static COMM_STATUS COMM_CommandProcess(struct COMM *self)
{
    switch(self->cmd)
    {
        case COMM_COMMAND_ECHO:
            self->tx_payload_len = self->rx_payload_len;
            memcpy(self->tx_payload_ptr, self->rx_payload_ptr, self->tx_payload_len);
            return COMM_STATUS_SUCCESS;
        
        default:
            self->status_msg = true;
            return COMM_STATUS_UNKNOWN_COMMAND;
    }
}

void __attribute__((weak)) COMM_Send(struct COMM *self)
{
    /* Fill in user application */
    printf("Simulate tx process\n");
    printf("TX buf (Size: %d):\n", self->tx_buf_rear);
    for(int i = 0; i < self->tx_buf_rear; i++)
    {
        if( i % 0x10 == 0)
        {
            printf("[0x%04X]: ", i);
        }

        printf("0x%02X ", self->tx_buf[i]);

        if( (i + 1) % 0x10 == 0)
        {
            printf("\n");
        }
    }
    printf("\n");

    printf("Checksum: %d\n", COMM_ChecksumCal(self->tx_buf, self->tx_buf_rear));
}

void COMM_Init(struct COMM *self, uint8_t *_rx_buf, uint16_t _rx_buf_size, uint8_t *_tx_buf, uint16_t _tx_buf_size)
{
    self->rx_buf            = _rx_buf;
    self->rx_buf_size       = _rx_buf_size;
    self->tx_buf            = _tx_buf;
    self->tx_buf_size       = _tx_buf_size;
    COMM_Reset(self);
}

void COMM_Receive(struct COMM *self, uint8_t *buf, uint16_t size)
{
    if( self->rx_buf_rear >= self->rx_buf_size )
    {
        printf("Rx buf overflow\n");
        return;
    }

    memcpy(&self->rx_buf[self->rx_buf_rear], buf, size);
    self->rx_buf_rear += size;
}

void COMM_Run(struct COMM *self)
{
    if( !self->rx_buf_rear )
    {
        return;
    }

    self->status = COMM_FindPacket(self);
    if( self->status != COMM_STATUS_SUCCESS )
    {
        printf("Err @ COMM_FindPacket\n");
        goto COMM_SEND_STATUS;
    }

    self->status = COMM_ChecksumCheck(self);
    if( self->status != COMM_STATUS_SUCCESS )
    {
        printf("ERR @ COMM_ChecksumCheck\n");
        goto COMM_SEND_STATUS;
    }

    self->status = COMM_CommandProcess(self);
    if( self->status_msg )
    {
        goto COMM_SEND_STATUS;
    }
    else
    {
        goto COMM_SEND_DATA;
    }

    COMM_SEND_STATUS:
    self->tx_payload_len    = 1;
    self->tx_payload_ptr[0] = (uint8_t) self->status;

    COMM_SEND_DATA:
    COMM_SendResponse(self);
    COMM_Reset(self);
}
