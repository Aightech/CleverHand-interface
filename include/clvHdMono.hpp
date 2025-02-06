#ifndef __CLV_HD_MONO_HPP__
#define __CLV_HD_MONO_HPP__

#include <algorithm> // for std::copy
#include <iostream>
#include <string>
#include <unordered_map>

#include <tcp_client.hpp>
#include <udp_client.hpp>

#define TCP_PORT 5000
#define UDP_PORT 12345

namespace ClvHd
{
class MonoController : virtual public ESC::CLI
{
    public:
    MonoController(int verbose = -1)
        : ESC::CLI(verbose, "monoclvhd"), serverTCP(TCP_PORT, 10, verbose - 1),
          serverUDP(UDP_PORT, 10, verbose - 1)
    {
        logln("created", true);
    };
    ~MonoController()
    {
        logln("destroyed", true);
        serverTCP.stop();
        serverUDP.stop();
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
    stop()
    {
        serverTCP.stop();
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
        // Communication::SOCKET s = *(Communication::SOCKET *)addr;
        // MonoController *clvhd = (MonoController *)data;
        (void)server;
        (void)buffer;
        (void)size;
        (void)addr;
        (void)data;
    };

    static void
    callbackUDP(Communication::Server *server,
                uint8_t *buffer,
                size_t size,
                void *addr,
                void *data)
    {
        (void)buffer;
        (void)size;
        (void)data;
        uint16_t port = TCP_PORT;
        // MonoController *clvhd = (MonoController *)data;
        Communication::UDPServer *serverUDP =
            (Communication::UDPServer *)server;
        serverUDP->send_data((uint8_t *)&port, sizeof(port), addr);
    };

    static void
    newClient(Communication::Server *server,
              void *addr,
              Communication::SOCKET s,
              void *data)
    {
        (void)server;
        (void)addr;
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
    clientThread(Communication::SOCKET s);

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
} // namespace ClvHd

#endif // __CLV_HD_MONO_HPP__