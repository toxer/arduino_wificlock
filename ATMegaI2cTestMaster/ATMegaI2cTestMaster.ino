#include <Wire.h>
#define DEVICE 8
void setup() {
  // put your setup code here, to run once:
  Wire.begin();

}

void loop() {
  // put your main code here, to run repeatedly:
    Wire.beginTransmission(DEVICE); // transmit to device #8
  Wire.write("ping signal ");        // sends five bytes
  Wire.endTransmission();    // stop transmitting
  delay(2000);

}
