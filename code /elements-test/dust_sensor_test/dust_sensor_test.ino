#include <GP2Y1010AU0F.h>

SharpDustSensor sensor = SharpDustSensor( 1, 2, A0 );

void setup() {
  Serial.begin( 9600 );
  sensor.begin();
}

void loop() {
  float density = sensor.getDensity();
  Serial.print( "density = " );
  Serial.println( density );
  delay( 1000 );
}
