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

    char *decbuf = new char[this->msg_length_decoded + 1];
    char *dec_msg = decode(in_bytes, decbuf);

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

    delete[] decbuf;
}

char *SerialConnection::decode(char *data, char *ret)
{

    Base64.decode(ret, data, this->msg_length_encoded);

    return ret;
}
/*
char *SerialConnection::__insert_initial_char(char *message, char *ret) const
{
    char *ret = new char[strlen(message) + 1];
    ret[0] = this->msg_start;
    for (int i = 0; i < strlen(message); i++)
    {
        ret[i + 1] = message[i];
    }
    return ret;
}
*/
char *SerialConnection::encode(char *data, char *ret)
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
    
    /*
    char *enc_input = "12345678AAAA";
    char *encoded_msg;
    Base64.encode(encoded_msg, enc_input, this->msg_length_decoded);
    */

    int encodedLength = Base64.encodedLength(this->msg_length_decoded);
    char encodedString[encodedLength];
    Base64.encode(encodedString, message_data, this->msg_length_decoded);

    String strmsg = encodedString;
    strmsg += '&';
    strcpy((char *)ret, strmsg.c_str());

    //encoded_msg[this->msg_length_encoded] = 38;

    return ret /*__insert_initial_char(encoded_msg)*/;
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