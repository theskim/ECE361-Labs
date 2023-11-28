# chmod +x server client server.sh client.sh
make
clear
echo "Testing /logout"
{
    echo "/login username password 0.0.0.0 3456"
    echo "/logout"
} | sudo ./client

echo "\nTesting  /login with same username"
{
    echo "/login username password 0.0.0.0 3456"
    sleep 1
    echo "/logout"
} | sudo ./client & \
{
    echo "/login username abcde 0.0.0.0 3456\n"
    sleep 1
    echo "/login username password 0.0.0.0 3456" 
    echo "/logout"
} | sudo ./client

echo "\nTesting /list (single user)"
{
    echo "/login username password 0.0.0.0 3456"
    echo "/list"
    echo "/logout"
} | sudo ./client

echo "\nTesting /list (multiple users)"
{
    echo "/login username password 0.0.0.0 3456"
    echo "/list"
    sleep 1
    echo "/logout"
} | sudo ./client & \
{
    echo "/login username2 password2 0.0.0.0 3456"
    echo "/list"
    sleep 1
    echo "/logout"
} | sudo ./client & \
{
     echo "/login username3 password3 0.0.0.0 3456"
     echo "/list"
     sleep 1
     echo "/logout"
} | sudo ./client