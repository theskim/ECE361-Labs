import random
import string

file_path = '../random_text.txt'
file_size = 1000 

random_text = ''.join(random.choice(string.ascii_letters) for _ in range(file_size))

with open(file_path, 'w') as file:
    file.write(random_text)