/* Author: Noa Sendlhofer - noa.sendlhofer@wpn.ch
 * Desc: Button class implementation
 */

#include "button/button.h"

Button::Button(std::shared_ptr<SerialConnection> &serial_connection, bool &g_status_flag, PORT port, char key)
{
    this->serial_connection = serial_connection;
    this->g_status_flag = &g_status_flag;
    this->port = port;
    this->key = key;

    pinMode(this->port, INPUT);
}

void Button::update()
{
    int status = digitalRead(this->port);

    if (status == HIGH && *this->g_status_flag)
    {
        this->serial_connection->send_button_message(this->key);
        *this->g_status_flag = false;
        this->_pressed = false;
    }
    else if (status == LOW && !this->_pressed)
    {
        *this->g_status_flag = true;
        this->_pressed = true;
    }
}