#include "Fletcher16.h"
#include "ElapsedMillis.h"
#include "OledHelper.h"
#include <string.h>
#include <limits.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SoftwareSerial.h>
#include <avr/sleep.h>


/******************************************************************************
* Enums
******************************************************************************/

enum ATCommandResultEnum {
	CMD_OK = 0,
	CMD_ERROR = 1,
	CMD_TIMEOUT = 2,
	CMD_UNEXPECTED = 3
}; 

typedef enum ATCommandResultEnum ATCommandResult;


/******************************************************************************
* Function Headers
******************************************************************************/
void sleepForever(void);
bool setupXmit(uint8_t setPin);
ATCommandResult localATCommand(uint8_t setPin, const char* command);
bool clearIncomingNoise(uint8_t tries, uint8_t msDelay);
uint8_t readHC12IntoBuffer(char* buffer, uint8_t index, uint8_t bufferSize, uint32_t readDelay);
void waitForHC12DataTries(unsigned long tries, uint32_t readDelay);

/******************************************************************************
* Constants and #defines
******************************************************************************/
#define HC12RxdPin					 4  // "RXD" Pin on HC12
#define HC12TxdPin					 5  // "TXD" Pin on HC12
#define HC12SetPin					 6  // "SET" Pin on HC12
#define BUTTON_PIN					 2  // the number of the pushbutton pin
#define LED_PIN						13	// the number of the LED pin

#define	AT_COMMAND_TIMEOUT_SECS		30	// Number of seconds before an AT command timeout
#define	AT_MODE_TRANSITION_DELAY	100	// Delay when going in to/out of AT command mode
#define BUFFER_SIZE					64	// The size of the input/output buffers.  Overkill, but hey
#define DELAY_READ					5	// Delay between reading individual bytes

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
#define OLED_RESET					-1		// Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS				0x3C	///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
#define SCREEN_WIDTH				128		// OLED display width, in pixels
#define SCREEN_HEIGHT				64		// OLED display height, in pixels


// variables will change:
ElapsedMillisClass* waitStartTimer = new ElapsedMillisClass();
ElapsedMillisClass* lastXmitTimer = new ElapsedMillisClass(millis() - 50000);
ElapsedMillisClass* lastTimeoutTimer = new ElapsedMillisClass(lastXmitTimer->getStart());

//volatile unsigned long waitStart = millis();
//volatile unsigned long lastXmitMillis = millis() - 50000;
//volatile unsigned long lastTimeoutMillis = lastXmitMillis;
volatile uint8_t buttonState = HIGH;  // variable for reading the pushbutton status
volatile bool waitingResponse = false;
char xmitBuffer[BUFFER_SIZE] = { 0 };
char recvBuffer[BUFFER_SIZE] = { 0 };
OledHelperClass *OledHelper = new OledHelperClass();

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
	OledHelper->init(&display);
	OledHelper->startup();
	OledHelper->splash("Hello");
	OledHelper->setCursor();
	OledHelper->splash("Ahmed!");

	if (setupXmit(HC12SetPin)) {
		Serial.println("HC-12 successfully initialized");
	}
	else {
		sleepForever();
	}
}

void loop() {
	// read the state of the pushbutton value:
	bool doTransmit = false;

	if (!waitingResponse) {
		unsigned long secondsSinceLastXmit = lastXmitTimer->elapsedSeconds();

		if (secondsSinceLastXmit >= 5) {
			// Wait at least 5 seconds before the next button push

			if (secondsSinceLastXmit >= 10) {
				OledHelper->clear();
			}

			uint8_t newButtonState = digitalRead(BUTTON_PIN);
			// check if the pushbutton is pressed. If it is, the buttonState is HIGH:
			if (newButtonState != buttonState) {
				Serial.println("newButtonState != buttonState");
				if (newButtonState == LOW) {
					Serial.println("newButtonState == LOW");
					// turn LED on:
					//      digitalWrite(LED_PIN, HIGH);
					doTransmit = true;
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

		if (doTransmit) {
			FletcherCheckBytes checkBytes;
			doTransmit = false;
			digitalWrite(LED_PIN, HIGH);

			OledHelper->printChars("XMIT", true, 0, 4);
			int bufferSize = sprintf(xmitBuffer, "millis %lu\r", millis());
			checkBytes.bothBytes16 = GetFletcher16CheckBytes((uint8_t *)xmitBuffer, bufferSize);
			xmitBuffer[bufferSize++] = checkBytes.checkbyte8[0];
			xmitBuffer[bufferSize++] = checkBytes.checkbyte8[1];
			xmitBuffer[bufferSize] = 0x00;

			Serial.print("XMITting buffer of ");
			Serial.print(bufferSize);
			Serial.print(" characters: ");
			Serial.println(xmitBuffer);

			for (int i = 0; i < bufferSize; i++) {
				HC12.write(xmitBuffer[i]);
			}
			HC12.flush();

			OledHelper->clear();
			digitalWrite(LED_PIN, LOW);

			waitStartTimer->setStart(millis());
			lastXmitTimer->setStart(millis());
			waitingResponse = true;
		}
		else {
			while (Serial.available()) {  // If Serial monitor has data
				HC12.write(Serial.read());  // Send that data to HC-12
			}
		}
	}


	/*------------------------------------------------------------------+
	|	Check to see if anything is incoming.							|
	|	Since we have tight control we shouldn't get unexpected data.	|
	|	- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -   |
	|	Ideally, at this point we would have some sort of "state" set	|
	|	so if we do get a response to a message we can process it.		|
	+------------------------------------------------------------------*/
	if (HC12.available()) {
		Serial.println("incoming data!");
		OledHelper->printlnChars("RECV", true, 0, 4);

		uint8_t bytesRead = readHC12IntoBuffer(recvBuffer, 0, BUFFER_SIZE, DELAY_READ);

		Serial.println(recvBuffer);
		OledHelper->printlnChars(recvBuffer, false, -1, -1);

		waitingResponse = false;
	}

	/*------------------------------------------------------------------+
	|	If we're waiting for a response check how long it's been.  If	|
	|	we've waited too long report it and stop waiting any longer.	|
	+------------------------------------------------------------------*/
	if (waitingResponse) {
		unsigned long elapsedTimeSeconds = waitStartTimer->elapsedSeconds();

		if (waitingResponse && (elapsedTimeSeconds > 5)) {
			Serial.print("Waiting! Waited this long in seconds: ");
			Serial.println(elapsedTimeSeconds);
			waitingResponse = false;
			OledHelper->printChars("FAILED TO ACK;", true, 0, 4);
			lastTimeoutTimer->start();
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


/// <summary>Waits for any unwanted characters to appear. Often occurs on startup before things settle.</summary>
/// <param name="tries">Number of tries</param>
/// <param name="msDelay">Number of milliseconds between tries</param>
/// <returns>true if noise, false otherwise</returns>
bool clearIncomingNoise(uint8_t tries, uint8_t msDelay) {
	waitForHC12DataTries(tries, msDelay);

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
	bool setupSucceeded;

	digitalWrite(setPin, LOW);			// Set HC-12 into AT Command mode
	delay(AT_MODE_TRANSITION_DELAY);	// Wait for the HC-12 to enter AT Command mode

	HC12.print("\r");					// Send <cr> to HC-12
	delay(100);
	clearIncomingNoise(10, 10);

	if (localATCommand(setPin, "AT\r") == CMD_OK) {
		Serial.println("Transmitter setup successful.");
		setupSucceeded = true;
	}
	else {
		Serial.println("Transmitter setup failed.");
		setupSucceeded = false;
	}

	digitalWrite(setPin, HIGH);			// Exit AT Command mode
	delay(AT_MODE_TRANSITION_DELAY);	// Wait for the HC-12 to enter AT Command mode

	return setupSucceeded;
}


void waitForHC12DataTimed(ElapsedMillisClass* timer, unsigned long maxWaitSeconds, uint32_t readDelay) {
	while (!HC12.available() && timer->elapsedSeconds() < maxWaitSeconds) {
		(readDelay == 0) ? void(0) : delay(readDelay);
	}
}


void waitForHC12DataTries(unsigned long tries, uint32_t readDelay) {
	unsigned long counter = 0;
	while (!HC12.available() && counter < tries) {
		(readDelay == 0) ? void(0) : delay(readDelay);
		counter++;
	}
}


ATCommandResult localATCommand(uint8_t setPin, const char* command) {
	const int tries = 10;
	const int msDelay = 10;
	ElapsedMillisClass* timer = NULL;

	HC12.print(command);  // Send command to HC-12
	timer = new ElapsedMillisClass();
	delay(100);

	waitForHC12DataTimed(timer, AT_COMMAND_TIMEOUT_SECS, msDelay);

	if (!HC12.available() && timer->elapsedSeconds() >= AT_COMMAND_TIMEOUT_SECS) {
		Serial.println("** Unable to receive response from AT command. **");
		return ATCommandResult::CMD_TIMEOUT;
	}

	int i = 0;
	while (HC12.available() && timer->elapsedSeconds() < AT_COMMAND_TIMEOUT_SECS && i < (BUFFER_SIZE - 1)) {	// If HC-12 has data (the AT Command response)
		char nextChar = HC12.read();																			// Get the next char from the HC-12
		recvBuffer[i++] = nextChar;
		delay(5);
	}
	recvBuffer[i] = 0;  // i is the length of the string

	Serial.print("\rReceived data after elapsed seconds: ");
	Serial.println(timer->elapsedSeconds());

	if (strncmp("OK", recvBuffer, 2) == 0) {
		Serial.println("\rCommand executed successfully.");
		return ATCommandResult::CMD_OK;
	}
	else if (strncmp("ERROR", recvBuffer, 5) == 0) {
		Serial.println("\rCommand failed.");
		return ATCommandResult::CMD_ERROR;
	}
	else {
		Serial.print("\rCommand returned unexpected results: '");
		Serial.print(recvBuffer);
		Serial.println("'");
		return ATCommandResult::CMD_UNEXPECTED;
	}
}


void sleepForever(void)
{
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);

	while (true)
	{
		sleep_enable();
		cli();
		sleep_cpu();
	}
}
