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
        delay(1);
    }
}

void SerialConnection::update()
{
    if (Serial.available() <= this->msg_length_encoded) { return; }

    char in_bytes[this->msg_length_encoded];

    int idx = 0;
    char current;
    bool fault = false;
    while (Serial.available())
    {
        current = Serial.read();
        if (current == this->msg_end)
        {
            break;
        }
        else if (idx < this->msg_length_encoded)
        {
            in_bytes[idx] = current;
        }
        else
        {
            fault = true;
        }
    }

    if (fault)
    {
        send_success_response();
        return;
    }

    unsigned char *dec_msg = decode(in_bytes);

    if (!verify_checksum(dec_msg))
    {
        send_error_response();
    }
    else
    {
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

    delete[] dec_msg;
}

unsigned char *SerialConnection::decode(char *data)
{
    unsigned char *ret = new unsigned char[this->msg_length_decoded + 1];
    Base64.decode(ret, data, this->msg_length_encoded);

    return ret;
}

char *SerialConnection::encode(char *data)
{
    CRC32 crc;
    uint32_t checksum = crc.calculate(data, this->msg_length_decoded - 4);

    unsigned char message_data[this->msg_length_decoded];
    for (int i = 0; i < this->msg_length_decoded - 4; i++)
    {
        message_data[i] = data[i];
    }
    
    message_data[this->msg_length_decoded - 4] = ((uint32_t)checksum >> 0) & 0xFF;
    message_data[this->msg_length_decoded - 3] = ((uint32_t)checksum >> 8) & 0xFF;
    message_data[this->msg_length_decoded - 2] = ((uint32_t)checksum >> 16) & 0xFF;
    message_data[this->msg_length_decoded - 1] = ((uint32_t)checksum >> 24) & 0xFF;
    
    int encodedLength = Base64.encodedLength(this->msg_length_decoded);
    char encodedString[encodedLength];
    Base64.encode(encodedString, message_data, this->msg_length_decoded);

    String strmsg = encodedString;
    strmsg += this->msg_end;

    char *ret = new char[this->msg_length_decoded + 1];
    strcpy((char *)ret, strmsg.c_str());

    return ret;
}

bool SerialConnection::verify_checksum(unsigned char *msg)
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