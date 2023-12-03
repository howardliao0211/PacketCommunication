# PacketCommunication
A C library for packet communication that can be easily ported into users' projects. 

**Note: This library only implements functions for the slave device.**

**A python example code has been included to show how the host device can communicate with the slave device.**

# User Guide
1. Declare a communication handle and initialize it with COMM_Init. 
```
struct COMM comm_handle;
COMM_Init(&comm_handle, rx_buf, BUF_SIZE, tx_buf, BUF_SIZE, example_write);
```

2. Use COMM_Receive to push the received packet into the rx buffer queue. This function could be placed in a interrupt callback or in the main task. 

3. Call COMM_Run in the main task to parse the received packet. User can create new command code in the comm.h and assign action for each specific command code in the COMM_CommandProcess function. 

# Packet Format
![image](https://github.com/howardliao0211/PacketCommunication/assets/129032373/d085ef07-8f4b-4130-b647-4df21aa7bacf)
