#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "comm.h"

#define BUF_SIZE         (1024)

uint8_t rx_buf[BUF_SIZE];
uint8_t tx_buf[BUF_SIZE];

uint8_t simulate_packet[BUF_SIZE];

uint16_t fill_simulate_packet(uint8_t *buf, uint16_t simulate_payload_num)
{
    uint16_t checksum;

    buf[0]                              = COMM_HEADER[0];
    buf[1]                              = COMM_HEADER[1];
    buf[2]                              = COMM_HEADER[2];
    buf[3]                              = COMM_HEADER[3];
    buf[COMM_DIR_OFFSET]                = COMM_DIR_WRITE;
    buf[COMM_COMMAND_OFFSET]            = COMM_COMMAND_ECHO;
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

int main(void)
{
    uint16_t tx_len;

    struct COMM comm_handle;
    COMM_Init(&comm_handle, rx_buf, BUF_SIZE, tx_buf, BUF_SIZE);

    tx_len = fill_simulate_packet(simulate_packet, 10);

    COMM_Receive(&comm_handle, simulate_packet, tx_len);
    COMM_Run(&comm_handle);

    return 1;
}
