/* Author: Noa Sendlhofer - noa.sendlhofer@wpn.ch
 * Desc: Base64 encoding and decoding
 */

#include "arduino_serial.h"

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
