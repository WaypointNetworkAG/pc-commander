// Author: Noa Sendlhofer

#include "arduino_serial.h"

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <thread>

#include "base64.h"

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "advapi32.lib")

ArduinoSerial::ArduinoSerial()
{
    this->serial = new serialib();

    get_available_COM_ports();

    std::thread GXDQThread;

    for (auto & available_com_port : this->available_com_ports)
    {
        std::cout << available_com_port << std::endl;

        char errorOpening = this->serial->openDevice(available_com_port, 115200);
        if (errorOpening == 1)
        {
            this->try_update = true;

            GXDQThread = std::thread(&ArduinoSerial::device_handshake, this);
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));

            std::cout << "Device open" << std::endl;
            this->try_update = false;
            if (this->connection_status.load() == STATUS_INITIALIZED)
            {
                std::cout << "Connected!" << std::endl;
                break;
            }
            else
            {
                GXDQThread.join();
            }
        }
        //this->serial->closeDevice();
    }

    if (this->connection_status.load() != STATUS_INITIALIZED)
    {
        this->g_status = STATUS_ERROR;
        std::cout << "No device found!" << std::endl;
    }
    else
    {
        this->g_status = STATUS_SUCCESS;
        std::cout << "Serial Constructor done!" << std::endl;
    }
}

ArduinoSerial::~ArduinoSerial()
{
    this->serial->closeDevice();
}

void ArduinoSerial::device_handshake()
{
    char *message = encode(const_cast<char *>(this->host_key));

    int ret = this->serial->writeString(message);
    std::cout << ret << std::endl;

    delete[] message;

    while (this->connection_status.load() != STATUS_INITIALIZED && this->try_update.load())
    {
        update();
    }
}

char *ArduinoSerial::encode(char *data)
{
    uint32_t checksum = CRC::Calculate(data, this->msg_length_decoded - 4, CRC::CRC_32());

    unsigned char message_data[this->msg_length_decoded];
    for (int i = 0; i < this->msg_length_decoded - 4; i++)
    {
        message_data[i] = data[i];
    }
    message_data[this->msg_length_decoded - 4] = ((uint32_t)checksum >> 0) & 0xFF;
    message_data[this->msg_length_decoded - 3] = ((uint32_t)checksum >> 8) & 0xFF;
    message_data[this->msg_length_decoded - 2] = ((uint32_t)checksum >> 16) & 0xFF;
    message_data[this->msg_length_decoded - 1] = ((uint32_t)checksum >> 24) & 0xFF;

    std::string result;
    base64::encode(result, message_data, this->msg_length_decoded);
    //result += '&';
    result.insert(0, 1, this->msg_end);

    char *ret = new char[this->msg_length_encoded + 2];
    strcpy((char *)ret, result.c_str());

    return ret;
}

unsigned char *ArduinoSerial::decode(char *data) const
{
    std::vector<std::uint8_t> ret_vec = base64::decode(data, this->msg_length_encoded);

    unsigned char *ret = new unsigned char[this->msg_length_decoded + 1];
    std::copy(ret_vec.begin(), ret_vec.end(), ret);

    return ret;
}

bool ArduinoSerial::verify_checksum(unsigned char *msg) const
{
    uint32_t checksum = CRC::Calculate(msg, this->msg_length_decoded - 4, CRC::CRC_32());

    uint32_t rec_checksum;

    rec_checksum = msg[this->msg_length_decoded - 1];
    rec_checksum = rec_checksum << 8;
    rec_checksum = rec_checksum | msg[this->msg_length_decoded - 2];
    rec_checksum = rec_checksum << 8;
    rec_checksum = rec_checksum | msg[this->msg_length_decoded - 3];
    rec_checksum = rec_checksum << 8;
    rec_checksum = rec_checksum | msg[this->msg_length_decoded - 4];

    return checksum == rec_checksum;
}

void ArduinoSerial::shutdown()
{
    HANDLE hToken;
    TOKEN_PRIVILEGES tkp;

    // Get a token for this process.

    if (!OpenProcessToken(GetCurrentProcess(),
                          TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
        return;

    // Get the LUID for the shutdown privilege.

    LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME,
                         &tkp.Privileges[0].Luid);

    tkp.PrivilegeCount = 1;  // one privilege to set
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    // Get the shutdown privilege for this process.

    AdjustTokenPrivileges(hToken, FALSE, &tkp, 0,
                          (PTOKEN_PRIVILEGES)NULL, 0);

    if (GetLastError() != ERROR_SUCCESS)
        return;

    // Shut down the system and force all applications to close.

    if (!ExitWindowsEx(EWX_SHUTDOWN | EWX_FORCE,
                       SHTDN_REASON_MAJOR_OPERATINGSYSTEM |
                       SHTDN_REASON_MINOR_UPGRADE |
                       SHTDN_REASON_FLAG_PLANNED))
        return;

    //shutdown was successful
    return;
}

bool ArduinoSerial::get_available_COM_ports()
{
    DWORD test;
    bool gotPort = 0; // in case the port is not found
    TCHAR COMPath[5000];

    for(int i = 0; i < 255; i++) // checking ports from COM0 to COM255
    {
        char number_str[10];
        sprintf(number_str, "%d", i);

        char *ComName = new char[strlen("COM") + strlen(number_str) + 1];
        strcpy(ComName,"COM");
        strcat(ComName,number_str);

        test = QueryDosDevice(ComName, (LPSTR)COMPath, 5000);

        // Test the return value and error if any
        if(test != 0) //QueryDosDevice returns zero if it didn't find an object
        {
            gotPort = 1; // found port
            this->available_com_ports.push_back(ComName);
        }

        if(::GetLastError() == ERROR_INSUFFICIENT_BUFFER)
        {
            COMPath[10000]; // in case the buffer got filled, increase size of the buffer.
            continue;
        }
    }

    return gotPort;
}

void ArduinoSerial::update()
{
    if (serial->available() <= this->msg_length_encoded) {
        return;
    }

    std::cout << "Response!" << std::endl;

    char* in_bytes = new char[this->msg_length_encoded];

    serial->readString(in_bytes, this->msg_end, 17);

    in_bytes[strlen(in_bytes) - 1] = '\0';

    unsigned char* dec_msg = decode(in_bytes);

    if (!verify_checksum(dec_msg))
    {
        if (this->connection_status.load() == STATUS_INITIALIZED)
        {
            send_error_response();
        }
    }
    else
    {
        char *message = new char[this->msg_length_decoded - 3];
        strncpy(message, reinterpret_cast<const char *>(dec_msg), 8);
        message[8] = '\0';

        if (strcmp(message, this->device_key) == 0)
        {
            this->connection_status = STATUS_INITIALIZED;
        }
        else if (strcmp(message, this->success_msg) == 0)
        {
            this->g_status = STATUS_SUCCESS;
            this->heartbeat_ack = true;
        }
        else if (strcmp(message, this->error_msg) == 0)
        {
            this->g_status = STATUS_ERROR;
        }
        else if (strcmp(message, this->button_a) == 0)
        {
            this->g_status = STATUS_SUCCESS;
            //TODO: Send Keystroke
            send_success_response();
        }
        else
        {
            this->g_status = STATUS_ERROR;
            send_error_response();
        }

        delete[] message;
    }

    delete[] dec_msg;
}