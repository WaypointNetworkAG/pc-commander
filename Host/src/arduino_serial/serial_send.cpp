/* Author: Noa Sendlhofer - noa.sendlhofer@wpn.ch
 * Desc: Send serial messages
 */

#include "arduino_serial.h"

void ArduinoSerial::send_error_response()
{
    char *message = encode(const_cast<char *>(this->error_msg));

    this->serial->writeString(message);

    delete[] message;
}

void ArduinoSerial::send_success_response()
{
    char *message = encode(const_cast<char *>(this->success_msg));

    this->serial->writeString(message);

    delete[] message;
}

void ArduinoSerial::send_heartbeat_message()
{
    char *message = encode(const_cast<char *>(this->heartbeat_msg));

    this->serial->writeString(message);

    delete[] message;
}