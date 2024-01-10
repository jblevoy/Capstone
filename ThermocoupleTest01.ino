#include <OneWire.h>
#include <DallasTemperature.h>

// Data wire is connected to the Arduino digital pin 4 for thermocouple
#define ONE_WIRE_BUS 8 

// Setup a oneWire instance to communicate with any OneWire devices (thermocouple)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature sensor (thermocouple)
DallasTemperature sensors(&oneWire);

float plate = 0;
float container = 0;

void setup() {
  Serial.begin(9600);
  // Start up the OneWire library
  sensors.begin();
}

void loop() {
  sensors.requestTemperatures();
  plate = sensors.getTempFByIndex(0);  //Chamber A - can has more than one IC on the same bus. 0 refers to the first IC on the wire
  Serial.println(plate);
  //Serial.println(plate);
  delay(1000);
  container = sensors.getTempFByIndex(1);  //Chamber A - can has more than one IC on the same bus. 0 refers to the first IC on the wire
  Serial.println(container);
  //Serial.println(plate);
  delay(1000);
}
