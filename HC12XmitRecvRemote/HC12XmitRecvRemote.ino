#include "ElapsedMillis.h"
#include "OledHelper.h"
#include <string.h>
#include <limits.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SoftwareSerial.h>



/******************************************************************************
* Function Headers
******************************************************************************/
bool setupXmit(uint8_t setPin);
bool localATCommand(uint8_t setPin, const char* command);
bool clearIncomingNoise(uint8_t msDelay, uint8_t tries);
uint8_t readHC12IntoBuffer(char* buffer, uint8_t index, uint8_t bufferSize, uint32_t readDelay);
void waitForHC12DataTimed(unsigned long* secondsWaited, unsigned long maxWaitSeconds, unsigned long startMillis, uint32_t readDelay);
void waitForHC12DataTries(unsigned long tries, uint32_t readDelay);
unsigned long elapsedMillis(unsigned long now, unsigned long earlier);
unsigned long elapsedSeconds(unsigned long now, unsigned long earlier);
unsigned long elapsedSecondsSince(unsigned long earlier);

/******************************************************************************
* Constants and #defines
******************************************************************************/
#define HC12RxdPin 4  // "RXD" Pin on HC12
#define HC12TxdPin 5  // "TXD" Pin on HC12
#define HC12SetPin 6  // "SET" Pin on HC12
#define BUTTON_PIN 2  // the number of the pushbutton pin
#define LED_PIN 13    // the number of the LED pin

#define DEBUGSERIAL

#ifdef DEBUGSERIAL
	#define DebugBegin(x) Serial.begin(x)
	#define DebugPrint(x) Serial.print(x)
	#define DebugPrintln(x) Serial.println(x)
	#define DebugFlush() Serial.flush()
#else
	#define DebugBegin(x)	/*Serial.begin(x);*/
	#define DebugPrint(x)	/*Serial.print(x);*/
	#define DebugPrintln(x) /*Serial.println(x);*/
	#define DebugFlush()	/*Serial.flush();*/
#endif

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library.
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...
#define OLED_RESET -1        // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C  ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
#define SCREEN_WIDTH 128     // OLED display width, in pixels
#define SCREEN_HEIGHT 64     // OLED display height, in pixels

#define BUFFER_SIZE 64  // The size of the input/output buffers.  Overkill, but hey
#define DELAY_READ 5    // Delay between reading individual bytes

// variables will change:
volatile unsigned long waitStart = millis();
volatile unsigned long lastXmitMillis = millis() - 50000;
volatile unsigned long lastTimeoutMillis = lastXmitMillis;
volatile uint8_t buttonState = HIGH;  // variable for reading the pushbutton status
volatile bool waitingResponse = false;
char xmitBuffer[BUFFER_SIZE] = { 0 };
char recvBuffer[BUFFER_SIZE] = { 0 };

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
SoftwareSerial HC12(HC12TxdPin, HC12RxdPin);  // HC-12 TX Pin, HC-12 RX Pin


void setup() {
	// initialize the LED pin as an output:
	// initialize the pushbutton pin as an input:
	pinMode(LED_PIN, OUTPUT);
	pinMode(HC12SetPin, OUTPUT);
	pinMode(BUTTON_PIN, INPUT_PULLUP);

	Serial.begin(9600);  // Serial port to computer
	HC12.begin(9600);    // Serial port to HC12

	if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
		Serial.println(F("SSD1306 allocation failed"));
	}

	// Show initial display buffer contents on the screen --
	OledHelper.startup();

	Serial.println("Starting up");

	setupXmit(HC12SetPin);
	Serial.println("HC-12 successfully initialized");
	digitalWrite(HC12SetPin, HIGH);  // Exit AT Command mode
	delay(100);                      // Wait for the HC-12 to enter AT Command mode

	OledHelper.init(&display);
	OledHelper.splash("Hello");

	unsigned long deltaSecs = ElapsedMillisClass::elapsedSeconds(120000L, 200000L);
	Serial.print("deltaSecs: ");
	Serial.println(deltaSecs);
}

void loop() {
	// read the state of the pushbutton value:
	bool doWork = false;

	if (!waitingResponse) {
		unsigned long secondsSinceLastXmit = elapsedSecondsSince(lastXmitMillis);

		if (secondsSinceLastXmit >= 5) {
			// Wait at least 5 seconds before the next button push

			if (elapsedSecondsSince(lastXmitMillis) >= 10 && elapsedSecondsSince(lastXmitMillis)) {
				OledHelper.clear();
			}

			uint8_t newButtonState = digitalRead(BUTTON_PIN);
			// check if the pushbutton is pressed. If it is, the buttonState is HIGH:
			if (newButtonState != buttonState) {
				Serial.println("newButtonState != buttonState");
				if (newButtonState == LOW) {
					Serial.println("newButtonState == LOW");
					// turn LED on:
					//      digitalWrite(LED_PIN, HIGH);
					doWork = true;
				}
				else {
					Serial.println("newButtonState == HIGH");
					// turn LED off:
					//      digitalWrite(LED_PIN, LOW);
				}
				delay(50);
				buttonState = HIGH;
			}
		}

		if (doWork) {
			doWork = false;
			digitalWrite(LED_PIN, HIGH);

			OledHelper.printChars("XMIT", true, 0, 4);
			int bufferSize = sprintf(xmitBuffer, "millis %lu\r", millis());

			Serial.print("XMITting buffer of ");
			Serial.print(bufferSize);
			Serial.print(" characters: ");
			Serial.println(xmitBuffer);

			for (int i = 0; i < bufferSize; i++) {
				HC12.write(xmitBuffer[i]);
			}
			HC12.flush();

			OledHelper.clear();
			digitalWrite(LED_PIN, LOW);

			waitStart = millis();
			lastXmitMillis = waitStart;
			waitingResponse = true;
		}
		else {
			while (Serial.available()) {  // If Serial monitor has data
				HC12.write(Serial.read());  // Send that data to HC-12
			}
		}
	}

	if (HC12.available()) {
		Serial.println("incoming data!");

		OledHelper.printlnChars("RECV", true, 0, 4);

		uint8_t bytesRead = readHC12IntoBuffer(recvBuffer, 0, BUFFER_SIZE, DELAY_READ);

		Serial.println(recvBuffer);

		OledHelper.printlnChars(recvBuffer, false, -1, -1);

		waitingResponse = false;
	}

	if (waitingResponse) {
		unsigned long currentMillis = millis();
		unsigned long elapsedTimeSeconds = elapsedSeconds(currentMillis, waitStart);

		if (waitingResponse && (elapsedTimeSeconds > 5)) {
			Serial.print("Waiting! Waited this long in seconds: ");
			Serial.println(elapsedTimeSeconds);
			waitingResponse = false;
			OledHelper.printChars("FAILED TO ACK;", true, 0, 4);
			lastTimeoutMillis = currentMillis;
		}
	}
}


uint8_t readHC12IntoBuffer(char* buffer, uint8_t index, uint8_t bufferSize, uint32_t readDelay) {
	uint8_t i = index;
	while (HC12.available() && i < (bufferSize - 1)) {  // If HC-12 has data
		buffer[i++] = HC12.read();
		(readDelay == 0) ? (void)0 : delay(5);
	}
	buffer[i] = 0;
	return i - index;
}


bool clearIncomingNoise(uint8_t msDelay, uint8_t tries) {
	waitForHC12DataTries(tries, msDelay);
	// int i = 0;
	// while (!HC12.available() && i < tries) {
	//   delay(msDelay);
	//   i++;
	// }

	if (!HC12.available()) {
		return false;
	}
	else {
		Serial.println();
		Serial.print("Noise begin>");
		while (HC12.available()) {
			Serial.write(HC12.read());
			delay(5);
		}
		Serial.println("<Noise end");
		return true;
	}
}


bool setupXmit(uint8_t setPin) {
	digitalWrite(setPin, LOW);  // Set HC-12 into AT Command mode
	delay(100);                 // Wait for the HC-12 to enter AT Command mode

	HC12.print("\r");  // Send <cr> to HC-12
	delay(100);
	clearIncomingNoise(10, 10);

	if (localATCommand(setPin, "AT\r")) {
		Serial.println("Transmitter setup successful.");
	}
	else {
		Serial.println("Transmitter setup failed.");
	}

	digitalWrite(setPin, HIGH);  // Exit AT Command mode
	delay(100);                  // Wait for the HC-12 to enter AT Command mode
}


void waitForHC12DataTimed(unsigned long* secondsWaited, unsigned long maxWaitSeconds, unsigned long startMillis, uint32_t readDelay) {
	unsigned long waitTime = *secondsWaited;
	while (!HC12.available() && waitTime < maxWaitSeconds) {
		(readDelay == 0) ? void(0) : delay(readDelay);
		waitTime = elapsedSeconds(millis(), startMillis);
	}
	*secondsWaited = waitTime;
}


void waitForHC12DataTries(unsigned long tries, uint32_t readDelay) {
	unsigned long counter = 0;
	while (!HC12.available() && counter < tries) {
		(readDelay == 0) ? void(0) : delay(readDelay);
		counter++;
	}
}


bool localATCommand(uint8_t setPin, const char* command) {
	const int tries = 10;
	const int msDelay = 10;
	unsigned long startTime;
	unsigned long secondsWaiting;

	HC12.print(command);  // Send command to HC-12
	startTime = millis();
	delay(100);

	secondsWaiting = elapsedSeconds(millis(), startTime);
	waitForHC12DataTimed(&secondsWaiting, 30, startTime, msDelay);

	if (!HC12.available() && secondsWaiting >= 30) {
		Serial.println("** Unable to receive response from AT command. **");
		return false;
	}

	int i = 0;
	while (HC12.available() && secondsWaiting < 30 && i < (BUFFER_SIZE - 1)) {  // If HC-12 has data (the AT Command response)
		char nextChar = HC12.read();                                              // Get the next char from the HC-12
		Serial.write(nextChar);                                                   // Send the data to Serial monitor
		recvBuffer[i++] = nextChar;
		delay(5);
		secondsWaiting = elapsedSeconds(millis(), startTime);
	}
	recvBuffer[i] = 0;  // i is the length of the string

	if (strncmp("OK", recvBuffer, 2) == 0) {
		Serial.println("\rCommand executed successfully.");
		return true;
	}
	else if (strncmp("ERROR", recvBuffer, 5) == 0) {
		Serial.println("\rCommand failed.");
		return false;
	}
	else {
		Serial.print("\rCommand returned unexpected results: '");
		Serial.print(recvBuffer);
		Serial.println("'");
		return false;
	}
}


unsigned long elapsedMillis(unsigned long now, unsigned long earlier) {
	if (now >= earlier) {
		return now - earlier;
	}
	else {
		return now + (ULONG_MAX - earlier);
	}
}


unsigned long elapsedSeconds(unsigned long now, unsigned long earlier) {
	return elapsedMillis(now, earlier) / 1000;
}


unsigned long elapsedSecondsSince(unsigned long earlier) {
	return elapsedMillis(millis(), earlier) / 1000;
}
