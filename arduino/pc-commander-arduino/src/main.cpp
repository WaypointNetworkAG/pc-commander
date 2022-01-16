#include <Arduino.h>
#include <avr/wdt.h>

#include "serial_connection/serial_connection.h"

SerialConnection *serial_connection;

void setup()
{
  serial_connection = new SerialConnection();

  wdt_enable(WDTO_8S);
}

void loop()
{ 
    serial_connection->update();
    delay(1);
}