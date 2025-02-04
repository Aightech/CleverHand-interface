#include <iostream>
#include <fstream>
#include <string>
#include <serial_client.hpp>
#include <strANSIseq.hpp>

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <serial_port> <credentials_file> verboselevel" << std::endl;
        return 1;
    }
    int verbose = 1;
    if (argc >= 4) {
        verbose = std::stoi(argv[3]);
    }
    ESC::CLI cli(verbose,"MAIN");
    std::ifstream file(argv[2]);
    cli.logln("Opening file: " + std::string(argv[2]), true);
    // Check if the file was opened successfully
    if (!file.is_open()) {
        std::cerr << cli.log_error("Error opening file") << std::endl;
        return 1;
    }
    //first line is the ssid and the second line is the password
    std::string ssid, password;
    std::getline(file, ssid);
    std::getline(file, password);
    cli.logln("SSID: " + ssid, true);
    // cli.logln("Password: " + password, true);
    
    cli.logln("Opening serial connection: " + std::string(argv[1]), true);
    Communication::Serial serial(verbose-1);
    serial.open_connection(argv[1], 115200);
    uint8_t size_ssid = ssid.size();
    uint8_t size_password = password.size();
    uint8_t cmd = 'C';
    serial.writeS(&cmd, 1);
    serial.writeS(&size_ssid, 1);
    serial.writeS(&size_password, 1);
    serial.writeS(ssid.c_str(), size_ssid);
    serial.writeS(password.c_str(), size_password);
    cli.logln("Credentials sent", true);

    

    return 0;
}
