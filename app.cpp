#include "application.h"
#include "SparkFun_Photon_Weather_Shield_Library.h"

float humidity = 0;
float tempf = 0;
float pascals = 0;
float inhg = 0;
float baroTemp = 0;
float altf = 0;

int iterations = 0;

char publishString[64];

//Create Instance of HTU21D or SI7021 temp and humidity sensor and MPL3115A2 barrometric sensor
Weather sensor;

void initWeather() {
  //Initialize the I2C sensors and ping them
  sensor.begin();

  sensor.setModeBarometer();

  sensor.setOversampleRate(7);

  sensor.enableEventFlags();
}

void endWeather()
{
  sensor.end();
}

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
// iterations before the value accurate
float readPressure() {
  float pa = sensor.readPressure();
  // try again value is incorrect
  if (pa < 10.00) {
    return readPressure();
  }
  return pa;
}

void getWeather()
{
  // Measure Relative Humidity from the HTU21D or Si7021
  humidity = sensor.getRH();

  // Measure Temperature from the HTU21D or Si7021
  tempf = sensor.getTempF();
  // Temperature is measured every time RH is requested.
  // It is faster, therefore, to read it from previous RH
  // measurement with getTemp() instead with readTemp()
  pascals = readPressure();

  //Measure the Barometer temperature in F from the MPL3115A2
  //baroTemp = readBaro();

  //If in altitude mode, you can get a reading in feet  with this line:
  //altf = readAltitude();
}

int getAndPublishWeather(String command)
{
  iterations++;

  initWeather();

  Serial1.print("getAndPublishWeather() iteration: ");
  Serial1.println(iterations);

  // turn on D7 led while taking measurements
  digitalWrite(D7, HIGH);

  getWeather();

  // TODO: still missing sprintf floats
  //sprintf(publishString, "{\"temp\": %.2f, \"hum\": %.2f, \"pa\": %.2f}", tempf, humidity, pascals);
  sprintf(publishString, "{\"temp\": %d, \"hum\": %d, \"pa\": %d}", (int)(tempf*100), (int)(humidity*100), (int)(pascals*100));

  Serial1.println(publishString);
  Serial1.print("tempf:"); Serial1.println(tempf);
  Serial1.print("hum:"); Serial1.println(humidity);
  Serial1.print("pa:"); Serial1.println(pascals);

  if (Particle.connected()) {
    Particle.publish("weather", publishString);
  } else {
    Serial1.println("Unable to publish weather, no cloud connection.");
  }

  endWeather();

  digitalWrite(D7, LOW);

  return 1;
}

void saveWeatherData()
{
  Serial1.println("saveWeatherData()");
}

//---------------------------------------------------------------
void setup()
{
  Serial1.begin(9600);

  Serial1.println("setup()");

  pinMode(D7, OUTPUT);

  Particle.function("getWeather", getAndPublishWeather);
}

//---------------------------------------------------------------
void loop()
{
  const unsigned long sampleInterval = 1 * 60 * 1000UL;
  static unsigned long lastSampleTime = 0 - sampleInterval;  // initialize such that a reading is due the first time through loop()

  unsigned long now = millis();
  if (now - lastSampleTime >= sampleInterval)
  {
    lastSampleTime += sampleInterval;
    // add code to take temperature reading here
    getAndPublishWeather("");
    saveWeatherData();
  }

  System.sleep(SLEEP_MODE_DEEP);
}


