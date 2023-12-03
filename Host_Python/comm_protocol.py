from typing import Protocol
import serial

class COMM_Protocol(Protocol):
    """
    A protocol class that specify how the COMM class send and receive packet. 
    """
    def send(self, data: list) -> bool:
        """
        An abstract function for sending packet. 
        Return true if the send function success. 
        """
        pass

    def receive(self, data: list) -> bool:
        """
        An abstract function for receiving packet. 
        Return true if the receive function success. 
        """
        pass

class COMM_Simulate(COMM_Protocol):
    
    def __init__(self) -> None:
        super().__init__()

    def send(self, data: list) -> bool:
        self.length = len(data)
        self.data = data

        print('Simulate Send')
        for num in data:
            print(f'{num: X} ', end='')
        print('')

        return True
    
    def receive(self, data: list) -> bool:
        data.clear()
        data.extend(list(self.data))
        return True

class COMM_Serial(COMM_Protocol):

    def __init__(self, serial: serial.Serial) -> None:
        self.serial = serial
    
    def __del__(self) -> None:
        self.serial.close()
    
    def send(self, data: list) -> bool:
        if not self.serial.is_open:
            return False
        
        self.serial.write(bytes(data))
        return True

    def receive(self, data: list) -> bool:
        if not self.serial.is_open:
            return False

        if self.serial.in_waiting == 0:
            self.serial.read(self.serial.in_waiting)
            return False

        data.clear()
        data.extend(list(self.serial.read(self.serial.in_waiting)))
