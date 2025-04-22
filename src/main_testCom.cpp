// pc_server.cpp
// Build (Linux): g++ -std=c++11 pc_server.cpp -o pc_server
#ifdef _WIN32
  #include <winsock2.h>
  #pragma comment(lib,"ws2_32.lib")
  using socklen_t = int;
#else
  #include <arpa/inet.h>
  #include <netinet/tcp.h>
  #include <sys/socket.h>
  #include <unistd.h>
  #define INVALID_SOCKET -1
  #define SOCKET_ERROR   -1
  using SOCKET = int;
#endif

#include <chrono>
#include <cstring>
#include <iostream>

int main(int argc, char* argv[]) {
    bool noDelay = false, quickAck = false;
    for(int i=1;i<argc;i++){
        if(std::strcmp(argv[i],"--nodelay")==0)  noDelay=true;
        if(std::strcmp(argv[i],"--quickack")==0) quickAck=true;
    }

#ifdef _WIN32
    WSADATA wsa; WSAStartup(MAKEWORD(2,2), &wsa);
#endif

    SOCKET srv = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr{ };
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(5000);
    bind(srv,(sockaddr*)&addr,sizeof(addr));
    listen(srv,1);

    std::cout<<"Waiting for ESP32..."<<std::endl;
    SOCKET cli = accept(srv,nullptr,nullptr);

    if(noDelay){
        int flag=1;
        setsockopt(cli, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag));
    }
    if(quickAck){
        int flag=1;
        setsockopt(cli, IPPROTO_TCP, TCP_QUICKACK, &flag, sizeof(flag));
    }

    for(int i=0;i<10;i++){
        // send a timestamp to the ESP32
        auto now = std::chrono::high_resolution_clock::now();
        uint64_t now_us = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count();
        uint32_t ts_pc = htonl(now_us);
        uint32_t ts_esp32 = 0;
        std::cout<<"Sending timestamp: "<< now_us << " µs"<<std::endl;
        int r = send(cli, (char*)&ts_pc, sizeof(ts_pc), MSG_NOSIGNAL);
        if(r<=0) break;
        std::cout<<"Sent timestamp: "<< now_us << " µs"<<std::endl;
        //wait for a timestamp from the ESP32
        r = recv(cli, (char*)&ts_esp32, sizeof(ts_esp32), MSG_WAITALL);
        std::cout<<"Received timestamp: "<< ntohl(ts_esp32) << " µs"<<std::endl;
        if(r<=0) break;

        auto now2 = std::chrono::high_resolution_clock::now();
        uint64_t now2_us = std::chrono::duration_cast<std::chrono::microseconds>(now2.time_since_epoch()).count();
        std::cout<<"Round-trip latency: "<< (now2_us - now_us) <<" µs"<<std::endl;


        // uint32_t ts_net;
        // int r = recv(cli, (char*)&ts_net, sizeof(ts_net), MSG_WAITALL);
        // if(r<=0) break;
        // uint32_t ts = ntohl(ts_net);
        // auto now = std::chrono::high_resolution_clock::now();
        // uint64_t now_us = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count();
        // std::cout<<"One‑way latency: "<< (now_us - ts) <<" µs"<<std::endl;
    }

    close(cli);
    close(srv);
#ifdef _WIN32
    WSACleanup();
#endif
    return 0;
}
