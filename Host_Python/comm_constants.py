from enum import Enum

COMM_HEADER                                     = "COMM"
COMM_DIR_WRITE                                  = 0
COMM_DIR_READ                                   = 1

COMM_HEADER_SIZE                                = 4
COMM_DIR_SIZE                                   = 1
COMM_COMMAND_SIZE                               = 1
COMM_PAYLOAD_LEN_SIZE                           = 2
COMM_CHK_SIZE                                   = 1
COMM_MIN_SIZE                                   = COMM_HEADER_SIZE + COMM_DIR_SIZE + COMM_COMMAND_SIZE + COMM_PAYLOAD_LEN_SIZE + COMM_CHK_SIZE

COMM_HEADER_OFFSET                              = 0
COMM_DIR_OFFSET                                 = 4
COMM_COMMAND_OFFSET                             = 5
COMM_PAYLOAD_LEN_OFFSET                         = 6
COMM_PAYLOAD_OFFSET                             = 8

COMM_HEADER_END                                 = COMM_HEADER_OFFSET + COMM_HEADER_SIZE
COMM_DIR_END                                    = COMM_DIR_OFFSET + COMM_DIR_SIZE
COMM_COMMAND_END                                = COMM_COMMAND_OFFSET + COMM_COMMAND_SIZE
COMM_PAYLOAD_LEN_END                            = COMM_PAYLOAD_LEN_OFFSET + COMM_PAYLOAD_LEN_SIZE

COMM_COMMAND_ECHO                               = 0x01

class COMM_STATUS(Enum):
    FAIL            = 0,
    SUCCESS         = 1,
    FORMAT_ERROR    = 0xE1,
    LENGTH_ERROR    = 0xE2,
    CHECKSUM_FAIL   = 0xE3,
    UNKNOWN_COMMAND = 0xE4,
