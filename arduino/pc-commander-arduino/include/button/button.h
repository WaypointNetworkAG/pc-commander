/* Author: Noa Sendlhofer - noa.sendlhofer@wpn.ch
 * Desc: Button class header
 */

#ifndef PC_COMMANDER_ARDUINO_BUTTON_
#define PC_COMMANDER_ARDUINO_BUTTON_

#include "serial_connection/serial_connection.h"
#include <ArxSmartPtr.h>

typedef int PORT;

class Button
{
public:
    Button(std::shared_ptr<SerialConnection> &serial_connection, bool &g_status_flag, PORT port, char key);
    void update();

private:
    std::shared_ptr<SerialConnection> serial_connection;

    PORT port;
    char key;

    bool _pressed;
    bool *g_status_flag;
};

#endif //PC_COMMANDER_ARDUINO_BUTTON_