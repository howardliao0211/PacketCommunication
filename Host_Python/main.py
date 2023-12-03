from comm_protocol import COMM_Serial, COMM_Simulate
from comm import COMM
import serial
import serial.tools.list_ports as list_ports

DEVICE_VID                      = 483
DEVICE_PID                      = 5740

def find_device_port() -> str:
    for port, desc, hwid in list_ports.comports():
        if (str(DEVICE_PID) in hwid) and (str(DEVICE_VID) in hwid):
            print('Device Port: {}'.format(port))
            return port
    return ''

if __name__ == '__main__':
    port = find_device_port()
    serial = serial.Serial(port=port, baudrate=921600)
    comm = COMM(COMM_Serial(serial), 1000)
    comm.echo(range(10))
    
