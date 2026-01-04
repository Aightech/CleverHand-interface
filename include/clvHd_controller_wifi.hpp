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

class WifiClient
{
    public:
    WifiClient(Communication::SOCKET s) : socket(s) {}

    Communication::SOCKET socket;
    std::thread *thread;
    bool init = false;
};

class WifiController : public Controller
{
    public:
    WifiController(int verbose = -1)
        : ESC::CLI(verbose, "ClvHd-Controller-wifi"),
          serverTCP(TCP_PORT, 10, verbose - 1),
          serverUDP(UDP_PORT, 10, verbose - 1)
    {
        serverTCP.disable_nagle();
        serverTCP.disable_quickack();

        logln("created", true);
    };
    ~WifiController()
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
        for(auto &client : m_clients) { client->thread->join(); }
        for(auto &client : m_clients)
        {
            delete client->thread;
            delete client;
        }
        logln("stopped", true);
    };

    virtual void
    setRGB(int id_module, RGBColor &color)
    {
        (void)id_module;
        (void)color;
    }

    uint8_t
    setup()
    {
        logln("Setup controller board", true);
        if(m_isStreaming)
            stop_stream();
        sendCmd('s');
        uint8_t nb = 0;
        readReply(&nb);
        logln("Number of modules found: " + std::to_string(nb), true);
        return nb;
    };

    virtual void
    stream(uint32_t mask_id,
           uint8_t n_cmd,
           uint8_t *cmd,
           uint8_t size,
           uint32_t period_us = 1000) override
    {
        if(m_isStreaming)
            stop_stream();
        uint8_t msg[10];
        *(uint32_t *)msg = mask_id;
        *(uint32_t *)(msg + 4) = period_us;
        logln("streaming period: " + std::to_string(period_us), true);
        msg[8] = size;
        msg[9] = n_cmd;
        m_isStreaming = true;
        sendCmd('R');
        sendCmd(msg, 10);
        sendCmd(cmd, n_cmd);
    };

    void
    stop_stream()
    {
        uint8_t msg[6];
        *(uint32_t *)msg = 0;
        msg[4] = 0;
        msg[5] = 0;
        sendCmd('R');
        sendCmd(msg, 6);
        m_isStreaming = false;
        // clear all clients dequeued data
        for(auto &cl : m_clients) serverTCP.clear_fifo(cl->socket);
    };

    virtual int
    sendCmd(uint8_t cmd, WifiClient *client = nullptr)
    {
        return sendCmd(&cmd, 1, client);
    };

    /**
     * @brief sendCmd Send a command to the given client.
     *
     * @param data Data to send.
     * @param size Size of the data to send.
     * @param clientSocket Id of the client to send the data to. If -1, send to all clients.
     * @return int Number of bytes sent.
     */
    virtual int
    sendCmd(uint8_t *data, size_t size, WifiClient *client = nullptr)
    {
        if(client == nullptr)
        {
            for(auto &cl : m_clients)
            {
                serverTCP.send_data(data, size, cl->socket);
            }
        }
        else
        {
            serverTCP.send_data(data, size, client->socket);
        }
        return size;
    };

    /**
     * @brief readReply Read a reply from the controller board. The reply contains a timestamp, a size and the data.
     *
     * @param buff Buffer to store the data.
     * @param timestamp Timestamp of the reply.
     * @param clientSocket Id of the client to read from. If -1, read from all clients.
     * @return int Number of bytes read.
     */
    virtual int
    readReply(uint8_t *buff,
              uint64_t *timestamp = nullptr,
              WifiClient *client = nullptr)
    {
        int n = 0; // if clientSocket == -1, read from all clients
        //a bit weird: stack received data in buff
        for(auto &cl : m_clients)
        {
            if(client != nullptr && cl != client)
                continue; // skip if we are not reading from the right client
            // logln("Reading from client: " + std::to_string(cl->socket), true);
            // Read the timestamp and the size of the data (8 bytes + 1 byte)
            // logln("Reading from client: " + std::to_string(cl->socket), true);
            serverTCP.is_available(cl->socket); // check if data is available
            // logln("Data available: " + std::to_string(n_available), true);
            serverTCP.read_byte(cl->socket, m_buffer, 9,
                                true); //blocking read
            // logln("Read " + std::to_string(m_buffer[8]) + " bytes", true);
            // logln("Read " + std::to_string(m) + " bytes", true);
            // logln("have to Read " + std::to_string(m_buffer[8]) + " bytes",
            //       true);

            if(timestamp != nullptr)
                *timestamp = *(uint64_t *)m_buffer;

            serverTCP.read_byte(cl->socket, buff + n, m_buffer[8],
                                true); //blocking read
            if(client != nullptr && cl == client)
                break; // stop reading if we read from the right client
            n += m_buffer[8];
        }
        return n;
    }

    static void
    callbackTCP(Communication::Server *server,
                uint8_t *buffer,
                size_t size,
                void *addr,
                void *data)
    {
        // Communication::SOCKET s = *(Communication::SOCKET *)addr;
        // WifiController *clvhd = (WifiController *)data;
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
        // WifiController *clvhd = (WifiController *)data;
        std::string ip = std::string(
            inet_ntoa(((Communication::SOCKADDR_IN *)addr)->sin_addr));
        uint16_t port = ntohs(((Communication::SOCKADDR_IN *)addr)->sin_port);

        server->logln("UDP callback: sending data to port " +
                          std::to_string(port) + " from " + ip,
                      true);
        Communication::UDPServer *serverUDP =
            (Communication::UDPServer *)server;
        uint16_t msg = TCP_PORT;
        serverUDP->send_data((uint8_t *)&msg, sizeof(msg), addr);
    };

    static void
    newClient(Communication::Server *server,
              void *addr,
              Communication::SOCKET s,
              void *data)
    {
        (void)server;
        (void)addr;
        WifiController *clvhd = (WifiController *)data;
        clvhd->logln("New client#" + std::to_string(s), true);
        // clvhd->addClient(s);
        WifiClient *client = new WifiClient(s);
        clvhd->addClient(client);
    };

    void
    addClient(WifiClient *client)
    {
        client->thread =
            new std::thread(&WifiController::clientThread, this, client);
        uint8_t size = 0;
        uint8_t c = 0;
        serverTCP.read_byte(client->socket, &c, 1, true);
        serverTCP.read_byte(client->socket, (uint8_t *)&size, 1, true);
        uint8_t *buff = new uint8_t[size + 1];
        serverTCP.read_byte(client->socket, buff, size, true);
        buff[size] = '\0';
        logln("Client#" + std::to_string(client->socket) +
                  " Mac address: " + std::string((char *)buff),
              true);
        delete[] buff;
        m_clients.push_back(client);
        logln("Client#" + std::to_string(client->socket) +
                  " added to the list of clients",
              true);
    };

    void
    clientThread(WifiClient *client);

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
        if(!m_isStreaming)
        { //no need to send the command if we are streaming
            uint8_t msg[6];
            *(uint32_t *)msg = mask_id;
            msg[4] = size;
            msg[5] = n_cmd;
            sendCmd('r');
            sendCmd(msg, 6);
            sendCmd(cmd, n_cmd);
        }
        int n = readReply((uint8_t *)buff, timestamp);
        // logln("ReadM " + std::to_string(n) + " bytes", true);
        return n;
    };

    virtual int
    writeCmd_multi(uint32_t mask_id,
                   uint8_t n_cmd,
                   uint8_t *cmd,
                   uint8_t size = 0,
                   const void *data = nullptr) override
    {
        uint8_t msg[6];
        *(uint32_t *)msg = mask_id;
        msg[4] = size;
        msg[5] = n_cmd;
        sendCmd('w');
        sendCmd(msg, 6);
        sendCmd(cmd, n_cmd);
        if(size > 0)
            return sendCmd((uint8_t *)data, size);
        else
            return 0;
    };

    private:
    std::vector<WifiClient *> m_clients;
    Communication::TCPServer serverTCP;
    Communication::UDPServer serverUDP;
    uint8_t m_buffer[CLVHD_BUFFER_SIZE];
    bool m_isStreaming = false;
};
} // namespace ClvHd

#endif // __CLV_HD_MONO_HPP__