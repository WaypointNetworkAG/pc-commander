#ifndef PC_COMMANDER_ARDUINO_SERIAL_CONNECTION_
#define PC_COMMANDER_ARDUINO_SERIAL_CONNECTION_

#include "CRC32.h"
#include <Base64.h>

class SerialConnection
{
public:
    SerialConnection();
    void update();
    void send_button_message(char button_char);

private:
    char *decode(char *data);
    char *encode(char *data);
    bool verify_checksum(char *msg);

    void send_success_response();
    void send_error_response();
    void send_handshake_response();

    char *__insert_initial_char(char *message) const;

    const char msg_start = 38;
    const int msg_length_encoded = 16;
    const int msg_length_decoded = 12;

    const char *host_key      = "T6z$}~B{";
    const char *device_key    = "GXB)5jbS";
    const char *success_msg   = "SUCCESS!";
    const char *error_msg     = "ERROR!--";
    const char *heartbeat_msg = "STATUS--";

    bool connected = false;
};

#endif //PC_COMMANDER_ARDUINO_SERIAL_CONNECTION_