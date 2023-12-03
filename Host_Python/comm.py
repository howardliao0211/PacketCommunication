from dataclasses import dataclass, field
from comm_constants import *
from comm_protocol import COMM_Protocol, COMM_Simulate
import time

@dataclass
class Packet:
    valid:          bool
    status_msg:     bool = False
    status:         COMM_STATUS = COMM_STATUS.SUCCESS
    header:         str  = field(default_factory=str)
    direction:      int  = 0
    command:        int  = 0
    payload_len:    int  = 0
    payload:        list = field(default_factory=list)
    checksum:       int  = 0
    packet_list:    list = field(default_factory=list)

    def is_checksum_valid(self) -> bool:
        return calculate_checksum8(self.packet_list) == 0
    
    def is_header_valid(self) -> bool:
        return self.header == COMM_HEADER
    
    def is_read_direction(self) -> bool:
        return self.direction == COMM_DIR_READ

@dataclass
class COMM:
    protocol:   COMM_Protocol
    timeout_ms: int

    def echo(self, echo_packet: list) -> bool:
        command     = COMM_COMMAND_ECHO
        payload_len = len(echo_packet)
        payload     = echo_packet[:]

        response = self.__start_transaction(command, payload_len, payload)
        if not response.valid:
            return False
        
        if response.status != COMM_STATUS.SUCCESS:
            print(f'COMM Fail. Status: {response.status}')
            return False
        
        print('Echo Payload: ')
        print(response.payload)
        print('Echo Packet')
        print(bytes(response.packet_list))

        return True

    def __start_transaction(self, cmd: int, payload_len: int, payload: list) -> Packet:
        rx_list = list()

        if not self.__send_packet(cmd, payload_len, payload):
            return Packet(False)

        elapsed_time_ms = 0
        current_time_ms = int(time.time() * 1000)

        while not self.__receive_packet(rx_list):
            elapsed_time_ms += int(time.time() * 1000) - current_time_ms
            current_time_ms  = int(time.time() * 1000)

            if elapsed_time_ms >= self.timeout_ms:
                print('Timeout')
                return Packet(False)

        packet = list_to_packet(rx_list)
        if not packet.is_header_valid() or not packet.is_read_direction():
            return Packet(True, True, COMM_STATUS.FORMAT_ERROR)
        
        if not packet.is_checksum_valid():
            return Packet(True, True, COMM_STATUS.CHECKSUM_FAIL)
        
        return packet
    
    def __receive_only_transaction(self) -> Packet:
        rx_list = list()
        elapsed_time_ms = 0
        current_time_ms = int(time.time() * 1000)

        while not self.__receive_packet(rx_list):
            elapsed_time_ms += int(time.time() * 1000) - current_time_ms
            current_time_ms  = int(time.time() * 1000)

            if elapsed_time_ms >= self.timeout_ms:
                print('Timeout')
                return Packet(False)

        packet = list_to_packet(rx_list)
        if not packet.is_header_valid() or not packet.is_read_direction():
            return Packet(True, True, COMM_STATUS.FORMAT_ERROR)
        
        if not packet.is_checksum_valid():
            return Packet(True, True, COMM_STATUS.CHECKSUM_FAIL)
        
        return packet

    def __send_packet(self, cmd: int, payload_len: int, payload: list) -> bool:
        checksum    = int()
        packet      = bytes()

        packet += COMM_HEADER.encode('utf-8')
        packet += COMM_DIR_WRITE.to_bytes(COMM_DIR_SIZE, byteorder='big')
        packet += cmd.to_bytes(COMM_COMMAND_SIZE, byteorder='big')
        packet += payload_len.to_bytes(COMM_PAYLOAD_LEN_SIZE, byteorder='big')
        packet += bytes(payload)

        checksum = calculate_checksum8(packet)
        checksum = 0x100 - checksum

        packet += checksum.to_bytes(1, byteorder='big')

        return self.protocol.send(list(packet))
    
    def __receive_packet(self, data: list) -> bool:
        return self.protocol.receive(data)

def calculate_checksum8(byte_array: list) -> int:
    checksum = sum(byte_array) % 256
    return checksum

def list_to_packet(data: list) -> Packet:
    packet = Packet(True)
    byte_data = bytes(data[:])
    packet.header       = byte_data[COMM_HEADER_OFFSET: COMM_HEADER_END].decode('utf-8')
    packet.direction    = int.from_bytes(byte_data[COMM_DIR_OFFSET: COMM_DIR_END], byteorder='big')
    packet.command      = int.from_bytes(byte_data[COMM_COMMAND_OFFSET: COMM_COMMAND_END], byteorder='big')
    packet.payload_len  = int.from_bytes(byte_data[COMM_PAYLOAD_LEN_OFFSET: COMM_PAYLOAD_LEN_END], byteorder='big')
    packet.payload      = data[COMM_PAYLOAD_OFFSET: COMM_PAYLOAD_OFFSET + packet.payload_len]
    packet.checksum     = data[COMM_PAYLOAD_OFFSET + packet.payload_len]
    packet.packet_list  = data[:]

    if packet.payload_len == 1:
        packet.status_msg = True
        packet.status = COMM_STATUS(int(packet.payload[0]))

    return packet

if __name__ == '__main__':
    comm = COMM(COMM_Simulate, 1000)
    comm.echo([i for i in range(10)])
