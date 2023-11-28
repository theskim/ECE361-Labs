# chmod +x server client server.sh client.sh
make
clear
echo "Testing logout"
{
    echo "/login username password 0.0.0.0 3002"
    echo "/logout"
} | sudo ./client

# echo "\nTesting list"
# {
#     echo "/login username password 0.0.0.0 3001"
#     echo "/list"
# } | sudo ./client