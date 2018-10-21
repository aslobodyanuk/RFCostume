
#include <SPI.h>
#include <RF24_config.h>
#include <RF24.h>
#include <printf.h>
#include <nRF24L01.h>

int A = 2;
int B = 3;
int C = 4;
int D = 5;
int E = 6;
int F = 7;
int G = 8;
int H = 19;
int I = 15;
int J = 16;

byte addresses[] = { 111111,222222 };
RF24 radio(9, 10);

int recieverAddress = 0;
byte responseData = 0;

#define RF_RECIEVE_LED_PIN 2
#define RF_RESET_LED 3

long _inLoopTime;
long _latestRecieveTime = millis();
const long _maxLatestRecieveTime = 1000;

struct channelsData {
	boolean testConnection;
	byte c1;
	byte c2;
	byte c3;
	byte c4;
	byte c5;
	byte c6;
	byte c7;
	byte c8;
	byte c9;
	byte c10;
};

channelsData currentData;

void setup()
{
	Serial.begin(115200);
	radioInit();
  
	pinMode(A, OUTPUT);
	pinMode(B, OUTPUT);
	pinMode(C, OUTPUT);
	pinMode(D, OUTPUT);
	pinMode(E, OUTPUT);
	pinMode(F, OUTPUT);
	pinMode(G, OUTPUT);
	pinMode(H, OUTPUT);
	pinMode(I, OUTPUT);
	pinMode(J, OUTPUT);

	pinMode(RF_RECIEVE_LED_PIN, OUTPUT);
	pinMode(RF_RESET_LED, OUTPUT);
}

void loop()
{
	if (radio.available()) {
		_inLoopTime = millis();
		while (radio.available()) {
			digitalWrite(RF_RECIEVE_LED_PIN, HIGH);
			radio.read(&currentData, sizeof(currentData));
			radio.writeAckPayload(1, &responseData, 1);
			if (millis() - _inLoopTime > _maxLatestRecieveTime) {
				restartRadio();
			}
			digitalWrite(RF_RECIEVE_LED_PIN, LOW);
		}
		_latestRecieveTime = millis();
	}

	if (millis() - _latestRecieveTime >= _maxLatestRecieveTime) {
		restartRadio();
		_latestRecieveTime = millis();
	}

	if (currentData.testConnection == false) {
		digitalWrite(A, currentData.c1);  // Pin 2
		digitalWrite(B, currentData.c2);  // Pin 3
		digitalWrite(C, currentData.c3);  // Pin 4
		digitalWrite(D, currentData.c4);  // Pin 5
		digitalWrite(E, currentData.c5);  // Pin 6
		digitalWrite(F, currentData.c6);  // Pin 7
		digitalWrite(G, currentData.c7);  // Pin 8
		digitalWrite(H, currentData.c8);  // Pin 9
		digitalWrite(I, currentData.c9);
		digitalWrite(J, currentData.c10);
	}
}

void radioInit() {
	pinMode(9, OUTPUT);
	digitalWrite(9, HIGH);
	pinMode(10, OUTPUT);
	digitalWrite(10, HIGH);
	radio.begin();
	delay(10);
	radio.setPALevel(RF24_PA_HIGH);
	radio.enableAckPayload();
	radio.enableDynamicPayloads();
	radio.openReadingPipe(1, addresses[recieverAddress]);
	radio.startListening();
	radio.writeAckPayload(1, &responseData, sizeof(responseData));
}

void restartRadio() {
	//Serial.println("st");
	digitalWrite(RF_RESET_LED, HIGH);
	radio.powerDown();
	radioInit();
	radio.powerUp();
	//Serial.println("end");
	digitalWrite(RF_RESET_LED, LOW);
}


