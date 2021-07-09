# Socket_example
 simple socket Programming(JAVA & C)

### Java socket
```
javac SocketServer.java
javac SocketClient.java

java SocketServer
java SocketClient
```

### C
Linux:
```
gcc SocketServer.c -o SocketServer.o
gcc SocketClient.c -o SocketClient.o

./SocketServer.o
./SocketClient.o
```

Windows:
```
gcc .\SocketServer.c -o server -l wsock32
gcc .\SocketClient.c -o client -l wsock32

.\server.exe
.\client.exe
```

