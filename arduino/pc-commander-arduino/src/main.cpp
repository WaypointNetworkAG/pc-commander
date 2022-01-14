#include <Arduino.h>

void setup() {
  Serial.begin(115200);
  while (!Serial)
  {
    delay(1);
  }
}

void loop() {
  delay(3000);
  Serial.write("return\n");
}