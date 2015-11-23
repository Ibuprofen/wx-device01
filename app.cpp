#include <application.h>
#include "libs/SparkFun_Photon_Weather_Shield_Library.h"
#include "libs/SD.h"

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

// set up variables using the SD utility library functions:
Sd2Card card;
SdVolume volume;
SdFile root;

// SPI pins
const uint8_t chipSelect = A2;
const uint8_t mosiPin = A5;
const uint8_t misoPin = A4;
const uint8_t clockPin = A3;


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

void sensorBegin() {
  Serial1.println("sensorBegin()");
  sensor.begin();
  sensor.setModeBarometer();
  sensor.setOversampleRate(7);
  sensor.enableEventFlags();
}

void sensorEnd() {
  Serial1.println("sensorEnd Wire.end()"); 
  //sensor.end();
  Wire.end();
}

void getWeather()
{
  sensorBegin();

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
  sensorEnd();
}

void initSD() {
  Serial1.println("initSD");

  if (!card.init(SPI_HALF_SPEED, chipSelect)) {
    Serial1.println("initialization failed. Things to check:");
    Serial1.println("* is a card is inserted?");
    Serial1.println("* Is your wiring correct?");
    Serial1.println("* did you change the chipSelect pin to match your shield or module?");
    return;
  } else {
    Serial1.println("Wiring is correct and a card is present.");
  }
}

void endSD() {
  Serial1.println("endSD SPI.end()");
  SPI.end();
  //card.end();
}

void infoSD() {
  // print the type of card
  Serial1.print("\nCard type: ");
  switch(card.type()) {
    case SD_CARD_TYPE_SD1:
      Serial1.println("SD1");
      break;
    case SD_CARD_TYPE_SD2:
      Serial1.println("SD2");
      break;
    case SD_CARD_TYPE_SDHC:
      Serial1.println("SDHC");
      break;
    default:
      Serial1.println("Unknown");
  }

  // Now we will try to open the 'volume'/'partition' - it should be FAT16 or FAT32
  if (!volume.init(card)) {
    Serial1.println("Could not find FAT16/FAT32 partition.\nMake sure you've formatted the card");
    return;
  }

  // print the type and size of the first FAT-type volume
  uint32_t volumesize;
  Serial1.print("\nVolume type is FAT");
  Serial1.println(volume.fatType(), DEC);
  Serial1.println();

  volumesize = volume.blocksPerCluster();    // clusters are collections of blocks
  volumesize *= volume.clusterCount();       // we'll have a lot of clusters
  volumesize *= 512;                         // SD card blocks are always 512 bytes
  volumesize /= 1024;
  Serial1.print("Volume size (Mbytes): ");
  volumesize /= 1024;
  Serial1.println(volumesize);

  Serial1.println("\nFiles found on the card (name, date and size in bytes): ");
  root.openRoot(volume);

  // list all files in the card with date and size
  root.ls(LS_R | LS_DATE | LS_SIZE);
}


void saveSample() {
  Serial1.println("saveSample()");

  initSD();
  infoSD(); // TODO: save the data
  endSD();

}


int sampleSensor(String command)
{
  iterations++;
  Serial1.print("sampleSensor() iteration: ");
  Serial1.println(iterations);

  getWeather();

  // TODO: still missing sprintf floats
  //sprintf(publishString, "{\"temp\": %.2f, \"hum\": %.2f, \"pa\": %.2f}", tempf, humidity, pascals);
  sprintf(publishString, "{\"temp\": %d, \"hum\": %d, \"pa\": %d}", (int)(tempf*100), (int)(humidity*100), (int)(pascals*100));  

  Serial1.println(publishString);
  Serial1.print("tempf:"); Serial1.println(tempf);
  Serial1.print("hum:"); Serial1.println(humidity);
  Serial1.print("pa:"); Serial1.println(pascals);

  saveSample();

  if (Particle.connected()) {
    Particle.publish("weather", publishString);
  } else {
    Serial1.println("Unable to publish weather, no cloud connection.");
  }

  return 1;
}

//---------------------------------------------------------------
void setup()
{
  Serial1.begin(9600);

  Serial1.println("Hello World");

  pinMode(D7, OUTPUT);

  Particle.function("getWeather", sampleSensor);
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
 
    digitalWrite(D7, HIGH);

    sampleSensor("");

    digitalWrite(D7, LOW);
  }

  System.sleep(SLEEP_MODE_DEEP);
}


