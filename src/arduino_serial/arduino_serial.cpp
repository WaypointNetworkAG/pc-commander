/* Author: Noa Sendlhofer - noa.sendlhofer@wpn.ch
 * Desc: Serial device
 */

#include "arduino_serial.h"

#include <iostream>
#include <thread>

ArduinoSerial::ArduinoSerial()
{
    this->serial = new serialib();

    WindowsActions::get_available_COM_ports();

    std::thread GXDQThread;

    for (auto & available_com_port : WindowsActions::available_com_ports)
    {
        char errorOpening = this->serial->openDevice(available_com_port, 115200);
        if (errorOpening == 1)
        {
            this->try_update = true;

            GXDQThread = std::thread(&ArduinoSerial::device_handshake, this);
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));

            this->try_update = false;
            if (this->connection_status.load() == STATUS_INITIALIZED)
            {
                GXDQThread.join();
                break;
            }
            else
            {
                GXDQThread.join();
            }
        }
        this->serial->closeDevice();
    }

    if (this->connection_status.load() != STATUS_INITIALIZED)
    {
        this->g_status = STATUS_ERROR;
        std::cout << "No device found!" << std::endl;
    }
    else
    {
        this->g_status = STATUS_SUCCESS;
        std::cout << "Device connected!" << std::endl;
    }
}

ArduinoSerial::~ArduinoSerial()
{
    std::cout << "Destructor called" << std::endl;
    this->serial->closeDevice();
}

void ArduinoSerial::device_handshake()
{
    char *message = encode(const_cast<char *>(this->host_key));

    this->serial->writeString(message);

    delete[] message;

    while (this->connection_status.load() != STATUS_INITIALIZED && this->try_update.load())
    {
        update();
    }
}