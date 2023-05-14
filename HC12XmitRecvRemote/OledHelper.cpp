// 
// 
// 

#include "OledHelper.h"

void OledHelperClass::init(Adafruit_SSD1306* withDisplay)
{
	_display = withDisplay;
}


void OledHelperClass::splash(char* buffer)
{
	if (!_display) return;

	_display->clearDisplay();
	_display->print(buffer);
	_display->display();
	for (int i = 0; i < 10; i++) {
		_display->dim(false);
		delay(250);
		_display->dim(true);
		delay(250);
	}
	_display->clearDisplay();
	_display->display();
}


void OledHelperClass::startup(bool showSplash = false) {
	if (!_display) return;

	if (showSplash) {
		// Show initial display buffer contents on the screen -- by default this is an Adafruit splash screen.
		_display->display();
		delay(500);  // Pause for 2 seconds
	}

	// Clear the buffer
	_display->clearDisplay();
	_display->display();
	_display->setTextSize(2);               // Normal 1:1 pixel scale
	_display->setTextColor(SSD1306_WHITE);  // Draw white text
}


/// <summary>Clear the buffer and display</summary>
void OledHelperClass::clear() {
	_display->clearDisplay();
	_display->display();
}


/// <summary>Display string on OLED display with a print().</summary>
/// <param name="buffer">Pointer to a C-string (NULL terminated)</param>
/// <param name="clear">If true => clear display before printing</param>
/// <param name="col">If not -1, use to set column</param>
/// <param name="row">If not -1, use to set row</param>
void OledHelperClass::printChars(const char* buffer, bool clear = false, int16_t col = -1, int16_t row = -1) {
	printlnCharsCommon(false, buffer, clear, col, row);
}



/// <summary>Display string on OLED display with a println().</summary>
/// <param name="buffer">Pointer to a C-string (NULL terminated)</param>
/// <param name="clear">If true => clear display before printing</param>
/// <param name="col">If not -1, use to set column</param>
/// <param name="row">If not -1, use to set row</param>
void OledHelperClass::printlnChars(const char* buffer, bool clear = false, int16_t col = -1, int16_t row = -1) {
	printlnCharsCommon(true, buffer, clear, col, row);
}


/// <summary>Display string on OLED display.</summary>
/// <param name="isPrintLn">If true => println, else => print</param>
/// <param name="buffer">Pointer to a C-string (NULL terminated)</param>
/// <param name="clear">If true => clear display before printing</param>
/// <param name="col">If not -1, use to set column</param>
/// <param name="row">If not -1, use to set row</param>
void OledHelperClass::printlnCharsCommon(bool isPrintLn, const char* buffer, bool clear = false, int16_t col = -1, int16_t row = -1) {
	if (!_display) return;

	if (clear) {
		_display->clearDisplay();
	}

	if (col != -1 && row != -1) {
		_display->setCursor((col == -1) ? _display->getCursorX() : col, (row == -1) ? _display->getCursorY() : row);
	}

	isPrintLn ? _display->println(buffer) : _display->print(buffer);
	_display->display();

	Serial.print("Display on oled: '");
	Serial.print(buffer);
	Serial.println("'");
}


/// <summary>Display string on OLED display.</summary>
/// <param name="col">use to set column</param>
/// <param name="row">use to set row</param>
void OledHelperClass::setCursor(int16_t col = 0, int16_t row = 0) {
	if (!_display) return;

	_display->setCursor(col, row);
}



