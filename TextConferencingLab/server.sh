# chmod +x server client server.sh client.sh
make
clear
sudo valgrind  --leak-check=full --show-leak-kinds=all ./server 4567