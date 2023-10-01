Question: Can we use string functions on messages?

As UDP sends data in the form of datagrams (bytes), we should receive data in char* as buffer, hence we can apply string functions such as strcmp or strcat afterwards.