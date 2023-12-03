#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include "comm.h"

#define BUF_SIZE         (1024)

uint8_t rx_buf[BUF_SIZE];
uint8_t tx_buf[BUF_SIZE];

uint8_t simulate_packet[BUF_SIZE];

uint16_t fill_simulate_packet(uint8_t *buf, uint8_t dir, uint8_t cmd, uint16_t simulate_payload_num)
{
    uint16_t checksum;

    buf[0]                              = COMM_HEADER[0];
    buf[1]                              = COMM_HEADER[1];
    buf[2]                              = COMM_HEADER[2];
    buf[3]                              = COMM_HEADER[3];
    buf[COMM_DIR_OFFSET]                = dir;
    buf[COMM_COMMAND_OFFSET]            = cmd;
    buf[COMM_PAYLOAD_LEN_OFFSET]        = (uint8_t) simulate_payload_num >> 8;
    buf[COMM_PAYLOAD_LEN_OFFSET + 1]    = (uint8_t) simulate_payload_num & 0xFF;

    for(int i = 0; i < simulate_payload_num; i++)
    {
        buf[COMM_PAYLOAD_OFFSET + i] = i;
    }

    checksum = 0;
    for(int i = 0; i < COMM_PAYLOAD_OFFSET + simulate_payload_num; i++)
    {
        checksum += buf[i];
    }
    checksum = ~(checksum) + 1;

    buf[COMM_PAYLOAD_OFFSET + simulate_payload_num] = checksum & 0xFF;

    return COMM_MIN_SIZE + simulate_payload_num;
}

void example_send(struct COMM *self)
{
    /* Fill in user application */
    printf("Simulate tx process\n");
    printf("TX buf (rear: %d):\n", self->tx_buf_rear);
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
}

int main(void)
{
    uint16_t tx_len;

    struct COMM comm_handle;
    COMM_Init(&comm_handle, rx_buf, BUF_SIZE, tx_buf, BUF_SIZE, example_send);

    /* Case 1. Packet with 10 payloads */
    tx_len = fill_simulate_packet(simulate_packet, COMM_DIR_WRITE, COMM_COMMAND_ECHO, 10);
    COMM_Receive(&comm_handle, simulate_packet, tx_len);
    COMM_Run(&comm_handle);
    assert(comm_handle.status == COMM_STATUS_SUCCESS);

    /* Case 2. Packet with 20 payloads */
    tx_len = fill_simulate_packet(simulate_packet, COMM_DIR_WRITE, COMM_COMMAND_ECHO, 20);
    COMM_Receive(&comm_handle, simulate_packet, tx_len);
    COMM_Run(&comm_handle);
    assert(comm_handle.status == COMM_STATUS_SUCCESS);

    /* Case 3. Packet with 50 payloads */
    tx_len = fill_simulate_packet(simulate_packet, COMM_DIR_WRITE, COMM_COMMAND_ECHO, 50);
    COMM_Receive(&comm_handle, simulate_packet, tx_len);
    COMM_Run(&comm_handle);
    assert(comm_handle.status == COMM_STATUS_SUCCESS);

    /* Case 4. Packet with wrong format. */
    tx_len = fill_simulate_packet(simulate_packet, 0xFF, COMM_COMMAND_ECHO, 50);
    COMM_Receive(&comm_handle, simulate_packet, tx_len);
    COMM_Run(&comm_handle);
    assert(comm_handle.status == COMM_STATUS_FORMAT_ERROR);

    /* Case 5. Packet with wrong length. */
    tx_len = fill_simulate_packet(simulate_packet, COMM_DIR_WRITE, COMM_COMMAND_ECHO, 50);
    COMM_Receive(&comm_handle, simulate_packet, tx_len - 10);
    COMM_Run(&comm_handle);
    assert(comm_handle.status == COMM_STATUS_LENGTH_ERROR);
    
    /* Case 6. Packet with wrong checksum. */
    tx_len = fill_simulate_packet(simulate_packet, COMM_DIR_WRITE, COMM_COMMAND_ECHO, 50);
    simulate_packet[5] = 0xE1;
    COMM_Receive(&comm_handle, simulate_packet, tx_len);
    COMM_Run(&comm_handle);
    assert(comm_handle.status == COMM_STATUS_CHECKSUM_FAIL);

    /* Case 7. Packet with unknown command. */
    tx_len = fill_simulate_packet(simulate_packet, COMM_DIR_WRITE, 0xFF, 50);
    COMM_Receive(&comm_handle, simulate_packet, tx_len);
    COMM_Run(&comm_handle);
    assert(comm_handle.status == COMM_STATUS_UNKNOWN_COMMAND);

    return 1;
}
