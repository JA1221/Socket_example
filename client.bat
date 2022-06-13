echo off
gcc -o SocketClient SocketClient.c -lwsock32
SocketClient.exe
pause