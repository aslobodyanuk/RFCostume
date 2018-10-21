#include <RF24_config.h>
#include <RF24.h>
#include <printf.h>
#include <nRF24L01.h>
#include <SPI.h>

#define CHANNELS_COUNT 20
byte addresses[] = { 111111,222222 };
RF24 radio(9, 10);
int incomingByte[CHANNELS_COUNT];
boolean readyToSent = false;
boolean arrayWritingState = false;
int curArrayIndex = -1;

boolean enableLedSignalIndication = true;
long _testConnectionTimer;
int ledPins[][3] = { { 2,3,4 },{ 8,18,7 } };
byte colorsArray[6][3] = { { 255,0,0 },{ 255,255,0 },{ 0,255,0 },{ 0,255,255 },{ 0,0,255 },{ 255,0,255 } };
byte colorsNumbers[6] = { 0,42,85,127,170,213 };
const int _numberOfTests = 10;
const long goodSignal = 600;
const long badSignal = 6000;
boolean _testSuccessfull = false;

struct ChannelsData {
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

struct TestingData {
	boolean isRecieved;
	long time;
};

ChannelsData currentData1;
ChannelsData currentData2;

#define RECIEVERS_COUNT 2
boolean _packetHasSent[RECIEVERS_COUNT];
long _timePacketSent[RECIEVERS_COUNT];
long _runtimeTestConnectionTimer[RECIEVERS_COUNT];
byte _recievedBytes[RECIEVERS_COUNT];

void setup()
{
	Serial.begin(115200);
	radioInit();

	pinMode(ledPins[0][0], OUTPUT);
	pinMode(ledPins[0][1], OUTPUT);
	pinMode(ledPins[0][2], OUTPUT);
	pinMode(ledPins[1][0], OUTPUT);
	pinMode(ledPins[1][1], OUTPUT);
	pinMode(ledPins[1][2], OUTPUT);

	_testConnectionTimer = millis();

	//Initialize runtime test timer array
	for (int counter = 0; counter < RECIEVERS_COUNT; counter++) {
		_runtimeTestConnectionTimer[counter] = millis();
		_packetHasSent[counter] = false;
	}
}

void loop()
{
	if (Serial.available())
	{
		//Serial.println("Aval");
		while (Serial.available()) {
			char readChar = Serial.read();
			if (readChar == 'S') {
				arrayWritingState = true;
				curArrayIndex = 0;
				continue;
			}
			if (readChar == 'E') {
				arrayWritingState = false;
				readyToSent = true;
				break;
			}
			if (arrayWritingState == true) {
				incomingByte[curArrayIndex] = readChar;
				curArrayIndex++;
				if (curArrayIndex >= CHANNELS_COUNT) {
					arrayWritingState = false;
					break;
				}	
			}
		}

		/*if (readyToSent) {
			for (int counter = 0; counter < CHANNELS_COUNT; counter++) {
				if (incomingByte[counter] == 49) {
					incomingByte[counter] = 1;
				}
				if (incomingByte[counter] == 48) {
					incomingByte[counter] = 0;
				}
			}
		}*/

		if (readyToSent == true) {
			currentData1.c1 = incomingByte[0];
			currentData1.c2 = incomingByte[1];
			currentData1.c3 = incomingByte[2];
			currentData1.c4 = incomingByte[3];
			currentData1.c5 = incomingByte[4];
			currentData1.c6 = incomingByte[5];
			currentData1.c7 = incomingByte[6];
			currentData1.c8 = incomingByte[7];
			currentData1.c9 = incomingByte[8];
			currentData1.c10 = incomingByte[9];
			currentData1.testConnection = false;

			currentData2.c1 = incomingByte[10];
			currentData2.c2 = incomingByte[11];
			currentData2.c3 = incomingByte[12];
			currentData2.c4 = incomingByte[13];
			currentData2.c5 = incomingByte[14];
			currentData2.c6 = incomingByte[15];
			currentData2.c7 = incomingByte[16];
			currentData2.c8 = incomingByte[17];
			currentData2.c9 = incomingByte[18];
			currentData2.c10 = incomingByte[19];
			currentData2.testConnection = false;

			sendPacket(0, currentData1);
			sendPacket(1, currentData2);
			readyToSent = false;

			//recieveTestConnectionAnswer();
			/*configureSignalLed(0, 0);
			configureSignalLed(1, 0);*/
		}
	}
	else {
		executeConnectionTest();
	}
}

void sendPacket(int addressIndex, struct ChannelsData data) {
	radio.stopListening();
	radio.openWritingPipe(addresses[addressIndex]);
	
	if (millis() - _runtimeTestConnectionTimer[addressIndex] >= 500) {
		//Serial.println("Test");
		_timePacketSent[addressIndex] = micros();
		_packetHasSent[addressIndex] = radio.write(&data, sizeof(data));

		if (radio.isAckPayloadAvailable()) {
			if (!radio.available()) {
				//Serial.print("Not recieved answer");
				configureSignalLed(addressIndex, 0);
			}
			else {
				while (radio.available()) {
					radio.read(&_recievedBytes[addressIndex], sizeof(_recievedBytes[addressIndex]));
				}
				if (_recievedBytes[addressIndex] == addressIndex) {
					long timeSincePacketSent = millis() - _timePacketSent[addressIndex];
					byte resultSignalQuality = map(timeSincePacketSent, goodSignal, badSignal, 255, 0);
					configureSignalLed(addressIndex, resultSignalQuality);
					/*Serial.print("Recieved answer ");
					Serial.println(resultSignalQuality);*/
				}
			}
		}
		_runtimeTestConnectionTimer[addressIndex] = millis();
	}
	else {
		//Serial.println("Sent");
		radio.write(&data, sizeof(data));
	}
}

void testConnection(int addressIndex) {
	if (millis() - _runtimeTestConnectionTimer[addressIndex] >= 500) {
		radio.stopListening();
		radio.openWritingPipe(addresses[addressIndex]);

		TestingData connTestArray[_numberOfTests];
		int testCounter = 0;

		ChannelsData data;
		data.testConnection = true;

		while (testCounter < _numberOfTests) {
			_timePacketSent[addressIndex] = micros();
			_packetHasSent[addressIndex] = radio.write(&data, sizeof(data));

			if (radio.isAckPayloadAvailable()) {
				if (!radio.available()) {
					connTestArray[testCounter].isRecieved = false;
					connTestArray[testCounter].time = micros() - _timePacketSent[addressIndex];
				}
				else {
					while (radio.available()) {
						radio.read(&_recievedBytes[addressIndex], sizeof(_recievedBytes[addressIndex]));
					}
					if (_recievedBytes[addressIndex] == addressIndex) {
						connTestArray[testCounter].isRecieved = true;
						connTestArray[testCounter].time = micros() - _timePacketSent[addressIndex];
					}
				}
			}
			else {
				connTestArray[testCounter].isRecieved = false;
				connTestArray[testCounter].time = micros() - _timePacketSent[addressIndex];
			}
			testCounter++;
		}
		
		boolean shouldCheck = false;
		for (int counter = 0; counter < _numberOfTests; counter++) {
			if (connTestArray[counter].isRecieved == true) {
				shouldCheck = true;
				break;
			}
		}

		if (shouldCheck == true) {
			_testSuccessfull = true;
			long averageTime = 0;
			byte minusResult = 0;
			byte valuePerBadSignal = 255 / _numberOfTests;
			int tempDividerCounter = 0;
			for (int counter = 0; counter < _numberOfTests; counter++) {
				if (connTestArray[counter].isRecieved == true) {
					averageTime = averageTime + connTestArray[counter].time;
					tempDividerCounter++;
				}
				else {
					minusResult = minusResult + valuePerBadSignal;
				}
			}

			if (averageTime != 0) {
				averageTime = averageTime / tempDividerCounter;
				byte resultSignalQuality = map(averageTime, goodSignal, badSignal, 255, 0);
				resultSignalQuality = resultSignalQuality - minusResult;
				configureSignalLed(addressIndex, resultSignalQuality);
			}
			else {
				configureSignalLed(addressIndex, 0);
			}
		}
		else {
			configureSignalLed(addressIndex, 0);
		}
		_runtimeTestConnectionTimer[addressIndex] = millis();
	}
}

void executeConnectionTest() {
	for (int counter = 0; counter < RECIEVERS_COUNT; counter++) {
		testConnection(counter);
	}
}

void configureSignalLed(int ledIndex, byte signalQuality) {
	if (enableLedSignalIndication) {
		for (int counter = 5; counter >= 0; counter--) {
			if (signalQuality >= colorsNumbers[counter]) {
				ledWrite(ledIndex, colorsArray[counter]);
				break;
			}
		}
	}
}

void ledWrite(int ledIndex, byte values[]) {
	digitalWrite(ledPins[ledIndex][0], values[0]);
	digitalWrite(ledPins[ledIndex][1], values[1]);
	digitalWrite(ledPins[ledIndex][2], values[2]);
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
	radio.openWritingPipe(addresses[0]);
	radio.startListening();
}

void restartRadio() {
	radio.powerDown();
	radioInit();
	radio.powerUp();
}