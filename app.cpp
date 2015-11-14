#include "application.h"
#include "HTU21D.h"

HTU21D htu = HTU21D();

void setup()
{
	Serial1.begin(9600);

	Serial1.println("HTU21D test");

	while(! htu.begin()){
	    Serial1.println("HTU21D not found");
	    delay(1000);
	}

	Serial1.println("HTU21D OK");
}

void loop()
{
	Serial1.println("===================");
	Serial1.print("Hum:"); Serial1.println(htu.readHumidity());
	Serial1.print("Temp:"); Serial1.println(htu.readTemperature());
	Serial1.println();

	delay(1000);
}
