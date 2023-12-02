#ifndef __COMM_H_
#define __COMM_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define COMM_HEADER                             "COMM"
#define COMM_DIR_WRITE                          (0)
#define COMM_DIR_READ                           (1)

#define COMM_HEADER_SIZE                        (4)
#define COMM_DIR_SIZE                           (1)
#define COMM_COMMAND_SIZE                       (1)
#define COMM_PAYLOAD_LEN_SIZE                   (2)
#define COMM_CHK_SIZE                           (1)
#define COMM_MIN_SIZE                           (COMM_HEADER_SIZE + COMM_DIR_SIZE + COMM_COMMAND_SIZE + COMM_PAYLOAD_LEN_SIZE + COMM_CHK_SIZE)

#define COMM_HEADER_OFFSET                      (0)
#define COMM_DIR_OFFSET                         (4)
#define COMM_COMMAND_OFFSET                     (5)
#define COMM_PAYLOAD_LEN_OFFSET                 (6)
#define COMM_PAYLOAD_OFFSET                     (8)

#define COMM_COMMAND_ECHO                       (0x01)

typedef enum _COMM_STATUS
{
    COMM_STATUS_FAIL    = 0,
    COMM_STATUS_SUCCESS = 1,

    COMM_STATUS_MISSING_HEADER  = 0xE1,
    COMM_STATUS_LENGTH_ERROR    = 0xE2,
    COMM_STATUS_CHECKSUM_FAIL   = 0xE3,
    COMM_STATUS_UNKNOWN_COMMAND = 0xE4,

}COMM_STATUS;

struct COMM
{
    COMM_STATUS status;
    uint8_t     *rx_buf;
    uint16_t    rx_buf_size;
    uint16_t    rx_buf_rear;

    uint8_t     *tx_buf;
    uint16_t    tx_buf_rear;
    uint16_t    tx_buf_size;

    uint16_t    header_idx;
    uint8_t     cmd;

    uint8_t     *rx_payload_ptr;
    uint16_t    rx_payload_len;

    uint8_t     *tx_payload_ptr;
    uint16_t    tx_payload_len;

    bool header_found;
    bool status_msg;
};

/* Fill in user application for these fucnctions. */
void COMM_Send(struct COMM *self);

/* Insert these functions into user project. */
void COMM_Init(struct COMM *self, uint8_t *_rx_buf, uint16_t _rx_buf_size, uint8_t *_tx_buf, uint16_t _tx_buf_size);
void COMM_Receive(struct COMM *self, uint8_t *buf, uint16_t size);
void COMM_Run(struct COMM *self);

#endif /* __COMM_H_ */
