#include "serial_connection/serial_connection.h"

void SerialConnection::send_handshake_response()
{
    char *message = (char *)this->device_key;

    char encoded_message[this->msg_length_encoded + 1];

    char *enc_message = encode(message, encoded_message);

    Serial.write(enc_message, this->msg_length_encoded + 1);
}

void SerialConnection::send_success_response()
{
    char *message = (char *)this->success_msg;

    char encoded_message[this->msg_length_encoded + 1];

    char *enc_message = encode(message, encoded_message);

    Serial.write(enc_message, this->msg_length_encoded + 1);
}

void SerialConnection::send_error_response()
{
    char *message = (char *)this->host_key;

    char encoded_message[this->msg_length_encoded + 1];

    char *enc_message = encode(message, encoded_message);

    Serial.write(enc_message, this->msg_length_encoded + 1);
}

void SerialConnection::send_button_message(char button_char)
{
    char message[8] = {button_char, '-', '-', '-', '-', '-', '-', '-'};

    char encoded_message[this->msg_length_encoded + 1];

    char *enc_message = encode(message, encoded_message);

    Serial.write(enc_message, this->msg_length_encoded + 1);
}