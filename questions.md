File Transfer Lab
Part 1
Question: Can we use string functions on messages?

As UDP sends data in the form of datagrams (bytes), we should receive data in char* as buffer, hence we can apply string functions such as strcmp or strcat afterwards.

Part 2
Question: How long is the measured round-trip time?
Around 0.0001 to 0.0003 seconds (0.1 - 0.3 ms)

Part 3
To check the address: hostname -I

Part 4 
One file is segmented into packets for transfer, and acknowledgement guaranteescorrect receipt of the file. If one packet from the client is lost, what will happen? Packet Loss from the Client leading to loss of data, we need to prevent this.

If an ACK/NACK packet is lost, what will happen?
In our implementation, it should lead to retransmission

For a timeout, how do we select the value of t1?