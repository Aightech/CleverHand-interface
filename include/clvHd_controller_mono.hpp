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

#include "clvHd_controller.hpp"

namespace ClvHd
{
class MonoController : public Controller
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

    virtual void setRGB(int id_module, RGBColor &color)
    {
        (void)id_module;
        (void)color;
    }


    uint8_t setup()
    {return 0;};

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


    virtual int
    readCmd_multi(uint32_t mask_id,
                  uint8_t n_cmd,
                  uint8_t *cmd,
                  uint8_t size,
                  const void *buff,
                  uint64_t *timestamp = nullptr) override
    {
        (void)mask_id;
        (void)n_cmd;
        (void)cmd;
        (void)size;
        (void)buff;
        (void)timestamp;
        return 0;
    };


    virtual int
    writeCmd_multi(uint32_t mask_id,
                   uint8_t n_cmd,
                   uint8_t *cmd,
                   uint8_t size = 0,
                   const void *data = nullptr) override
    {
        (void)mask_id;
        (void)n_cmd;
        (void)cmd;
        (void)size;
        (void)data;
        return 0;
    };

    private:
    std::unordered_map<Communication::SOCKET, std::thread> m_clients;
    Communication::TCPServer serverTCP;
    Communication::UDPServer serverUDP;
};
} // namespace ClvHd

#endif // __CLV_HD_MONO_HPP__