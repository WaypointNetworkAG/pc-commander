/* Author: Noa Sendlhofer - noa.sendlhofer@wpn.ch
 * Desc: CRC32 checksum
 */

#include "arduino_serial.h"

bool ArduinoSerial::verify_checksum(unsigned char *msg) const
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