#include <Arduino.h>
#include "serial_connection/serial_connection.h"

SerialConnection *serial_connection;

void setup()
{
  serial_connection = new SerialConnection();
}

void loop()
{ 
    serial_connection->update();
    delay(1);
}