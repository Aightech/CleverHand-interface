// cpp script listening on port 5000
#include <com_client.hpp>
#include <iostream>
#include <string>
#include <tcp_client.hpp>

#include <algorithm> // for std::copy
#include <udp_client.hpp>
#include <unordered_map>
#include <vector>

#include "clvHdController.hpp"

#define TCP_PORT 5000
#define UDP_PORT 12345

class MonoController : public ClvHd::Controller
{
    public:
    MonoController(int verbose = -1)
        : ESC::CLI(verbose, "monoclvhd"), serverTCP(TCP_PORT, 10, verbose),
          serverUDP(UDP_PORT, 10, verbose)
    {
        std::cout << "MonoController created" << std::endl;
        logln("created", true);
    };
    ~MonoController()
    {
        logln("destroyed", true);
        serverTCP.stop();
        serverUDP.stop();
    };

    void
    stop()
    {
        serverTCP.stop();
        std::cout << "Waiting for clients to join" << std::endl;
        serverUDP.stop();
        for(auto &client : m_clients) { client.second.join(); }
        logln("stopped", true);
    };

  
    

    static void
    callbackTCP(Communication::Server *server,
                uint8_t *buffer,
                size_t size,
                void *addr,
                void *data)
    {
        Communication::SOCKET s = *(Communication::SOCKET *)addr;
        MonoController *clvhd = (MonoController *)data;
        clvhd->logln("Received TCP[" + std::to_string(size) +
                         "]: " + std::string((char *)buffer, size),
                     true);
    };

    static void
    callbackUDP(Communication::Server *server,
                uint8_t *buffer,
                size_t size,
                void *addr,
                void *data)
    {
        uint16_t port = TCP_PORT;
        MonoController *clvhd = (MonoController *)data;
        clvhd->logln("Received UDP[" + std::to_string(size) +
                         "]: " + std::string((char *)buffer, size),
                     true);
        Communication::UDPServer *serverUDP = (Communication::UDPServer *)server;
        serverUDP->send_data((uint8_t *)&port, sizeof(port), addr);
    };

    static void
    newClient(Communication::Server *server, void *addr, Communication::SOCKET s, void *data)
    {
        MonoController *clvhd = (MonoController *)data;
        clvhd->logln("New client", true);
        clvhd->addClient(s);
    };

    void
    addClient(Communication::SOCKET s)
    {
        m_clients[s] = std::thread(&MonoController::clientThread, this, s);
    };

    void
    start()
    {
        try
        {
            serverTCP.set_callback(callbackTCP, this);
            serverTCP.set_callback_newClient(newClient, this);
            serverTCP.start();
            serverUDP.set_callback(callbackUDP, this);
            serverUDP.start();
        }
        catch(const std::string &e)
        {
            std::cerr << e << '\n';
        }
    };

    void
    clientThread(Communication::SOCKET s)
    {
        //check if the socket is still connected
        int error = 0;
        socklen_t len = sizeof(error);
        int retval = getsockopt(s, SOL_SOCKET, SO_ERROR, &error, &len);
        logln("Client Thread started [retval: " + std::to_string(retval) +
                  ", error: " + std::to_string(error) + "]",
              true);
        //while the socket is still connected
        while(retval == 0 && error == 0)
        {
            logln("Client (socket: " + std::to_string(s) + ") [available: " +
                      std::to_string(serverTCP.is_available(s)) + "]",
                  true);
            if(serverTCP.is_available(s))
            {
                uint8_t c = 0;
                serverTCP.read_byte(s, &c, 1);
                logln("Received: " + std::to_string(c), true);
                switch(c)
                {
                case 'A':
                    logln("Initial message received", true);
                    break;
                default:
                    logln("Unknown message received", true);
                    break;
                }
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            retval = getsockopt(s, SOL_SOCKET, SO_ERROR, &error, &len);
        }
        logln("Client disconnected", true);
    };


    int
    nbClients()
    {
        return m_clients.size();
    };

    private:
    std::unordered_map<Communication::SOCKET, std::thread> m_clients;
    Communication::TCPServer serverTCP;
    Communication::UDPServer serverUDP;


};



int
main()
{
    MonoController clvhd(4);
    clvhd.start();
    while(clvhd.nbClients() < 1)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    std::this_thread::sleep_for(std::chrono::seconds(5));
    std::cout << "Stop" << std::endl;
    clvhd.stop();


    return 0;
}