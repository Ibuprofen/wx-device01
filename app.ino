#include "application.h"
#include "SparkFun_Photon_Weather_Shield_Library.h"

bool ledOn = false;

float humidity = 0;
float tempf = 0;
float pascals = 0;
float inhg = 0;
float baroTemp = 0;
float altf = 0;

char publishString[64];

//Create Instance of HTU21D or SI7021 temp and humidity sensor and MPL3115A2 barrometric sensor
Weather sensor;

float readBaro() {
  sensor.setModeBarometer();
  return sensor.readBaroTempF();
}

float readAltitude() {
  sensor.setModeAltimeter();
  return sensor.readAltitudeFt();
}

// Measure Pressure from the MPL3115A2
// When device first boots it may take a hundred or so
// iterations before the value is > 0
float readPressure() {
  float pa = sensor.readPressure();
  // try again
  if (pa == 0.00) {
    return readPressure();
  }
  return pa;
}

void getWeather()
{
  Particle.publish("test01", "1");

  // Measure Relative Humidity from the HTU21D or Si7021
  //humidity = sensor.getRH();

  // Measure Temperature from the HTU21D or Si7021
  //tempf = sensor.getTempF();
  // Temperature is measured every time RH is requested.
  // It is faster, therefore, to read it from previous RH
  // measurement with getTemp() instead with readTemp()
  //pascals = readPressure();

  //Measure the Barometer temperature in F from the MPL3115A2
  //baroTemp = readBaro();

  //If in altitude mode, you can get a reading in feet  with this line:
  //altf = readAltitude();

  Particle.publish("test01", "2");
}

int getAndPublishWeather(String command)
{
  getWeather();

  //sprintf(publishString, "{\"temp\": %.2f, \"hum\": %.2f, \"pa\": %.2f}", tempf, humidity, pascals);

  //Particle.publish("weather", publishString);

  return 1;
}

//---------------------------------------------------------------
void setup()
{
  Serial1.begin(38400);

  pinMode(D7, OUTPUT);

  //Initialize the I2C sensors and ping them
  sensor.begin();

  sensor.setModeBarometer();

  sensor.setOversampleRate(7);

  sensor.enableEventFlags();

  Particle.function("getWeather", getAndPublishWeather);

}

//---------------------------------------------------------------
void loop()
{
  if (millis() % 1000 == 0 && ledOn) {
      digitalWrite(D7, LOW);
      ledOn = !ledOn;
  } else if (millis() % 1000 == 0) {
      digitalWrite(D7, HIGH);
      ledOn = !ledOn;
  }
  //System.sleep(SLEEP_MODE_DEEP);
}
