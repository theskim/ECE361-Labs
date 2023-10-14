import os

file_path = '../FileTransferLab/part3/test.bin'
file_size = 1024

random_data = os.urandom(file_size)

with open(file_path, 'wb') as file:
    file.write(random_data)