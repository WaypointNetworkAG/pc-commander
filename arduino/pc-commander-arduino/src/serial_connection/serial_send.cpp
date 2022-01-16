#include "serial_connection/serial_connection.h"

void SerialConnection::send_handshake_response()
{
    char *message = (char *)this->device_key;

    char *enc_message = encode(message);

    Serial.write(enc_message, this->msg_length_encoded + 1);

    delete[] enc_message;
}

void SerialConnection::send_success_response()
{
    char *message = (char *)this->success_msg;

    char *enc_message = encode(message);

    Serial.write(enc_message, this->msg_length_encoded + 1);

    delete[] enc_message;
}

void SerialConnection::send_error_response()
{
    char *message = (char *)this->error_msg;

    char *enc_message = encode(message);

    Serial.write(enc_message, this->msg_length_encoded + 1);

    delete[] enc_message;
}

void SerialConnection::send_button_message(char button_char)
{
    while (this->connected)
    {
        char message[8] = {button_char, '-', '-', '-', '-', '-', '-', '-'};

        char *enc_message = encode(message);

        Serial.write(enc_message, this->msg_length_encoded + 1);

        delete[] enc_message;

        this->message_ack = false;

        delay(500);

        update();

        if (this->message_ack = true)
        {
            return;
        }
    }
}