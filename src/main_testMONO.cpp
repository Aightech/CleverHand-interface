// cpp script listening on port 5000
#include <com_client.hpp>
#include <iostream>
#include <string>
#include <tcp_client.hpp>

#include <udp_client.hpp>
#include <vector>

void
callback(Communication::Server *server,
         uint8_t *buffer,
         size_t size,
         void *addr,
         void *data)
{
    uint16_t port = 5000;
    std::cout << "Received: " << std::string((char *)buffer, size) << std::endl;
    server->send_data((uint8_t *)&port, sizeof(port), addr);
}

void
tcpcb(Communication::Server *server,
      uint8_t *buffer,
      size_t size,
      void *addr,
      void *data)
{
    uint16_t port = 12345;
    std::cout << "Received TCP[" << std::dec << size << "]: ";
    for(int i = 0; i < size; i++)
    {
        std::cout << std::hex << (int)buffer[i] << " ";
    }
    std::cout << std::endl;
    // server->send_data((uint8_t *)&port, sizeof(port), addr);
}

int
main()
{
    try
    {
        Communication::TCPServer serverTCP(5000, 10, 3);
        serverTCP.set_callback(tcpcb);
        serverTCP.start();
        Communication::UDPServer serverUDP(12345, 10, 3);
        serverUDP.set_callback(callback);
        serverUDP.start();

        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
        int i=0;
        for(auto s :serverTCP.get_clients())
        {
            
            while(serverTCP.is_available(s))
            {
                uint32_t dt = 0;
                serverTCP.read_byte(s, (uint8_t *)&dt, sizeof(dt));
                std::cout << i++<< " Received TCP: " << std::dec << dt << std::endl;
            }
        }
    }
    catch(const std::string &e)
    {
        std::cerr << e << '\n';
    }

    return 0;
}