/*
 * BSD 3-Clause License
 *
 * Copyright (c) 2021, Noa Sendlhofer
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

// Author: Noa Sendlhofer

#include "arduino_serial.h"

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <thread>

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "advapi32.lib")

#define SERIAL_PORT "COM9"

ArduinoSerial::ArduinoSerial()
{
    this->serial = new serialib();

    get_available_COM_ports();

    std::thread GXDQThread;

    for (int i = 0; i < this->available_com_ports.size(); i++)
    {
        std::cout << this->available_com_ports[i] << std::endl;

        char errorOpening = serial->openDevice("COM9", 115200);
        if (errorOpening == 1)
        {
            std::cout << "Port COM9 opened" << std::endl;
            this->try_update = true;
            GXDQThread = std::thread(&ArduinoSerial::device_handshake, this);
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            std::cout << "Sleep done" << std::endl;
            this->try_update = false;
            if (this->connected.load())
            {
                std::cout << "Connected!" << std::endl;
                break;
            }
            else
            {
                GXDQThread.join();
            }
        }
    }
    /*
    char errorOpening = serial.openDevice(SERIAL_PORT, 115200);
    if (errorOpening!=1)
        std::cout << "Error Opening" << std::endl;
    else
    {
        printf ("Successful connection to %s\n",SERIAL_PORT);

        char* buf = new char[100];

        while(true)
        {
            serial.readString(buf, '\n', 100);
            std::cout << buf << std::endl;
            if (strcmp(buf, "return\n") == 0)
            {
                shutdown();
                break;
            }
        }
    }
     */
}

void ArduinoSerial::device_handshake()
{
    char *message = encode(const_cast<char *>(this->host_key));

    this->serial->writeString(message);

    while (!this->connected.load() && this->try_update.load())
    {
        update();
    }
}

char * ArduinoSerial::__insert_initial_char(char *message) const
{
    char* ret = new char[strlen(message) + 1];
    ret[0] = this->msg_start;
    for (int i = 0; i < strlen(message); i++)
    {
        ret[i + 1] = message[i];
    }
    return ret;
}

char *ArduinoSerial::encode(char *data) const
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
    result = base64::encode(message_data, this->msg_length_decoded);

    char* ret = __insert_initial_char(const_cast<char *>(result.c_str()));

    std::cout << ret << std::endl;

    return ret;
}

char *ArduinoSerial::decode(char *data)
{
    char* dec_string = new char[this->msg_length_decoded];

    std::vector<std::uint8_t> ret = base64::decode(data, this->msg_length_encoded);

    std::copy(ret.begin(), ret.end(),dec_string);

    return dec_string;
}

bool ArduinoSerial::verify_checksum(char *msg)
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
        std::cout << serial->available() << std::endl;
        return;
    }

    std::cout << "Serial response!" << std::endl;

    char in_bytes[this->msg_length_encoded];

    serial->readString(in_bytes, this->msg_start, 17);

    /*

    while (serial->available())
    {
        char* test;
        serial->readChar(test, 0);
        std::cout << "char read:" << std::endl;
        std::cout << test << std::endl;
        //in_bytes[0] = test;
        if (in_bytes[0] == this->msg_start)
        {
            break;
        }
    }

    if (serial->available() != this->msg_length_encoded) { return; }

    for (int n = 0; n < this->msg_length_encoded; n++)
    {
        serial->readChar(reinterpret_cast<char *>(in_bytes[n]), 0);
    }

     */

    std::cout << in_bytes << std::endl;

    char* dec_msg = decode(in_bytes);

    if (!verify_checksum(dec_msg))
    {
        if (connected) { /*send_error_response();*/ }
        return;
    }

    char message[this->msg_length_decoded - 4];
    for (int i = 0; i < this->msg_length_decoded - 4; i++)
    {
        message[i] = dec_msg[i];
    }

    if (strcmp(message, this->device_key) == 0)
    {
        this->connected = true;
    }
}
