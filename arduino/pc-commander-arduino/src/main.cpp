#include <Controllino.h>
#include <avr/wdt.h>
#include <ArxSmartPtr.h>

#include "serial_connection/serial_connection.h"
#include "button/button.h"

std::shared_ptr<SerialConnection> serial_connection;
bool g_status_flag = true;

Button *button1;
Button *button2;
Button *button3;
Button *button4;
Button *shutdown;

void setup()
{
  serial_connection = std::make_shared<SerialConnection>();
  button1 = new Button(serial_connection, g_status_flag, CONTROLLINO_A0, '1');
  button2 = new Button(serial_connection, g_status_flag, CONTROLLINO_A1, '2');
  button3 = new Button(serial_connection, g_status_flag, CONTROLLINO_A2, '3');
  button4 = new Button(serial_connection, g_status_flag, CONTROLLINO_A3, '4');
  shutdown = new Button(serial_connection, g_status_flag, CONTROLLINO_A4, 'P');

  wdt_enable(WDTO_8S);
}

void loop()
{ 
    serial_connection->update();
    button1->update();
    button2->update();
    button3->update();
    button4->update();
    shutdown->update();

    delay(1);
}