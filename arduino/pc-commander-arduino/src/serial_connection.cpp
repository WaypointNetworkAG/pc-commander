#include "serial_connection/serial_connection.h"

SerialConnection::SerialConnection()
{
    Serial.begin(115200);
    while (!Serial)
    {
        delay(1);
    }

    while(!this->connected)
    {
        update();
    }
}

void SerialConnection::update()
{
    if (Serial.available() <= this->msg_length_encoded) { return; }

    char in_bytes[this->msg_length_encoded];

    while (Serial.available())
    {
        in_bytes[0] = Serial.read();
        if (in_bytes[0] == this->msg_start)
        {
            break;
        }
    }
    for (int n = 0; n < this->msg_length_encoded; n++)
    {
        in_bytes[n] = Serial.read();
    }

    char* dec_msg = decode(in_bytes);

    if (!verify_checksum(dec_msg))
    {
        send_error_response();
        return;
    }
    
    char message[this->msg_length_decoded - 4];
    for (int i = 0; i < this->msg_length_decoded - 4; i++)
    {
        message[i] = dec_msg[i];
    }

    if (strcmp(message, this->host_key) == 0)
    {
        send_handshake_response();
        this->connected = true;
    } 
    else if (strcmp(message, this->heartbeat_msg) == 0)
    {
        send_success_response();
    }
}

char *SerialConnection::decode(char *data)
{
    char dec_string[this->msg_length_decoded];

    Base64.decode(dec_string, data, this->msg_length_encoded);

    return dec_string;
}

char *SerialConnection::__insert_initial_char(char *message) const
{
    char *ret = new char[strlen(message) + 1];
    ret[0] = this->msg_start;
    for (int i = 0; i < strlen(message); i++)
    {
        ret[i + 1] = message[i];
    }
    return ret;
}

char *SerialConnection::encode(char *data)
{
    CRC32 crc;
    uint32_t checksum = crc.calculate(data, this->msg_length_decoded - 4);

    char message_data[this->msg_length_decoded];
    for (int i = 0; i < this->msg_length_decoded - 4; i++)
    {
        message_data[i] = data[i];
    }
    message_data[this->msg_length_decoded - 4] = ((uint32_t)checksum >> 0) & 0xFF;
    message_data[this->msg_length_decoded - 3] = ((uint32_t)checksum >> 8) & 0xFF;
    message_data[this->msg_length_decoded - 2] = ((uint32_t)checksum >> 16) & 0xFF;
    message_data[this->msg_length_decoded - 1] = ((uint32_t)checksum >> 24) & 0xFF;

    char encoded_msg[this->msg_length_encoded + 1];
    Base64.encode(encoded_msg, message_data, this->msg_length_decoded);

    encoded_msg[this->msg_length_encoded] = 38;

    return encoded_msg /*__insert_initial_char(encoded_msg)*/;
}

bool SerialConnection::verify_checksum(char* msg)
{
    CRC32 crc;

    uint32_t checksum = crc.calculate(msg, this->msg_length_decoded - 4);

    uint32_t rec_checksum;

    rec_checksum = msg[this->msg_length_decoded - 1];
    rec_checksum = rec_checksum << 8;
    rec_checksum = rec_checksum | msg[this->msg_length_decoded - 2];
    rec_checksum = rec_checksum << 8;
    rec_checksum = rec_checksum | msg[this->msg_length_decoded - 3];
    rec_checksum = rec_checksum << 8;
    rec_checksum = rec_checksum | msg[this->msg_length_decoded - 4];

    return rec_checksum == checksum;
}