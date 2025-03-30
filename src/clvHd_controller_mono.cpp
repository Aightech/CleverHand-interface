// #include "clvHdMono.hpp"

// namespace ClvHd
// {
// void
// MonoController::clientThread(Communication::SOCKET s)
// {
//     //check if the socket is still connected
//     int error = 0;
//     socklen_t len = sizeof(error);
//     int retval = getsockopt(s, SOL_SOCKET, SO_ERROR, &error, &len);
//     logln("Client Thread started", true);
//     //while the socket is still connected
//     while(retval == 0 && error == 0)
//     {
//         if(serverTCP.is_available(s))
//         {
//             uint8_t c = 0;
//             serverTCP.read_byte(s, &c, 1);
//             switch(c)
//             {
//             case 'A':
//             {
//                 logln("Initial message received", true);
//                 uint8_t size = 0;
//                 serverTCP.read_byte(s, (uint8_t *)&size, 1, true);
//                 logln("Size of the mac address: " + std::to_string(size), true);
//                 uint8_t *buff = new uint8_t[size+1];
//                 serverTCP.read_byte(s, buff, size, true);
//                 buff[size]='\0';
//                 logln("Mac address: " + std::string((char *)buff), true);
//                 delete[] buff;
//                 // serverTCP.send_data(buff, 2, s);
//             }
//             break;
//             case 'R':
//             {
//                 uint16_t val = 0;
//                 uint8_t size = 0;
//                 serverTCP.read_byte(s, (uint8_t *)&size, 1, true);
//                 logln("Reply receive: " + std::to_string(size), true);
//                 for(int i = 0; i < size; i++)
//                 {
//                     serverTCP.read_byte(s, (uint8_t *)&val, 2, true);
//                     logln("Received value: " + std::to_string(val), true);
//                 }
//             }
//             break;
//             default:
//                 logln("Unknown message received: " + std::to_string((int)c) +
//                       " : " + std::to_string(c));
//                 break;
//             }
//         }

//         std::this_thread::sleep_for(std::chrono::milliseconds(100));
//         retval = getsockopt(s, SOL_SOCKET, SO_ERROR, &error, &len);
//     }
//     logln("Client disconnected", true);
// };
// } // namespace ClvHd