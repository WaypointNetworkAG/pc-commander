/* Author: Noa Sendlhofer - noa.sendlhofer@wpn.ch
 * Desc: Arduino Serial Interface
 */

#ifndef PC_COMMANDER_ARDUINO_SERIAL_H_
#define PC_COMMANDER_ARDUINO_SERIAL_H_

#include "windows_actions.h"
#include "serialib.h"
#include <vector>
#include <base64_rfc4648.hpp>
#include <CRC.h>
#include <atomic>

using base64 = cppcodec::base64_rfc4648;

typedef enum GLOBAL_STATUS_DEF
{

    STATUS_SUCCESS      = 0,
    STATUS_ERROR        = 1,
    STATUS_INITIALIZED  = 2,
    STATUS_IDLE         = 3

} GLOBAL_STATUS;

class ArduinoSerial : public WindowsActions
{
public:
    ArduinoSerial();
    ~ArduinoSerial();
    bool update();

    void send_heartbeat_message();

    std::atomic<GLOBAL_STATUS_DEF> g_status{STATUS_IDLE};
    std::atomic<GLOBAL_STATUS_DEF> connection_status{STATUS_IDLE};
    std::atomic<bool> heartbeat_ack{true};

private:
    serialib *serial;

    void device_handshake();
    void send_error_response();
    void send_success_response();

    unsigned char *decode(char *data) const;
    char *encode(char *data);
    bool verify_checksum(unsigned char *msg) const;

    const char msg_end = 38;
    const int msg_length_encoded = 16;
    const int msg_length_decoded = 12;

    const char *host_key      = "T6z$}~B{";
    const char *device_key    = "GXB)5jbS";
    const char *success_msg   = "SUCCESS!";
    const char *error_msg     = "ERROR!--";
    const char *heartbeat_msg = "STATUS--";

    const char *button_1      = "1-------";
    const char *button_2      = "2-------";
    const char *button_3      = "3-------";
    const char *button_4      = "4-------";
    const char *button_S      = "P-------";

    std::atomic<bool> try_update{false};
    std::atomic<bool> receive_block{false};
};

#endif //PC_COMMANDER_ARDUINO_SERIAL_H_
