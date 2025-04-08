# WiFi protocol

- feed wifi creaddential to the modules via serial
- modules advertise their presence via UDP packet broadcasting on port 12345
- the wifi controller create a UDP and TCP server
- the UDP server listens to the port 12345 and when it receive a packet from one of the modules, it replies with its available TCP port 
- the module receive this reply, save the ip of the server and read its TCP port
- the module establish a tcp connection with the controller server 
- the tcp server has a list of client coresponding to the set of 