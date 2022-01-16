/* Author: Noa Sendlhofer - noa.sendlhofer@wpn.ch
 * Desc: Receive serial messages
 */

#include "arduino_serial.h"

#include <iostream>
#include <ostream>

//TEST
bool send_ack = false;

bool ArduinoSerial::update()
{
    if (!serial->isDeviceOpen()) { return false; }
    if (serial->available() > 0) { this->receive_block = true; }
    if (serial->available() <= this->msg_length_encoded) { return true; }

    char* in_bytes = new char[this->msg_length_encoded];

    serial->readString(in_bytes, this->msg_end, 17);

    in_bytes[strlen(in_bytes) - 1] = '\0';

    unsigned char* dec_msg = decode(in_bytes);

    if (!verify_checksum(dec_msg))
    {
        std::cout << "Checksum failed test" << std::endl;
        send_ack = true;
        if (this->connection_status.load() == STATUS_INITIALIZED)
        {
            send_error_response();
            this->g_status = STATUS_ERROR;
        }
    }
    else
    {
        //send_ack = false;
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
        else if (strcmp(message, this->button_1) == 0)
        {
            this->g_status = STATUS_SUCCESS;
            WindowsActions::send_keystroke(0x31);
            send_success_response();
        }
        else if (strcmp(message, this->button_2) == 0)
        {
            this->g_status = STATUS_SUCCESS;
            WindowsActions::send_keystroke(0x32);
            send_success_response();
        }
        else if (strcmp(message, this->button_3) == 0)
        {
            this->g_status = STATUS_SUCCESS;
            WindowsActions::send_keystroke(0x33);
            send_success_response();
        }
        else if (strcmp(message, this->button_4) == 0)
        {
            this->g_status = STATUS_SUCCESS;
            WindowsActions::send_keystroke(0x34);
            send_success_response();
        }
        else if (strcmp(message, this->button_S) == 0)
        {
            this->g_status = STATUS_SUCCESS;
            WindowsActions::shutdown();
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
    this->receive_block = false;

    return true;
}