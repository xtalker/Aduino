/*

MY9291 LED Driver Arduino library 1.0.1
Based on the C driver by MaiKe Labs

Copyright (c) 2016 - 2026 MaiKe Labs
Copyright (c) 2017 - Xose Pérez (for the library)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "my9291.h"

#if ARDUINO_ARCH_ESP8266

    extern "C" {
        void os_delay_us(unsigned int);
    }

#elif ARDUINO_ARCH_AVR

    #define os_delay_us delayMicroseconds

#endif

void my9291::_di_pulse(unsigned int times) {
	unsigned int i;
	for (i = 0; i < times; i++) {
		digitalWrite(_pin_di, HIGH);
		digitalWrite(_pin_di, LOW);
	}
}

void my9291::_dcki_pulse(unsigned int times) {
	unsigned int i;
	for (i = 0; i < times; i++) {
		digitalWrite(_pin_dcki, HIGH);
		digitalWrite(_pin_dcki, LOW);
	}
}

void my9291::_set_cmd(my9291_cmd_t command) {

    unsigned char i;
	unsigned char command_data = *(unsigned char *) (&command);
	_command = command;

	// ets_intr_lock();
	// TStop > 12us.
	os_delay_us(12);
	// Send 12 DI pulse, after 6 pulse's falling edge store duty data, and 12
	// pulse's rising edge convert to command mode.
	_di_pulse(12);
	// Delay >12us, begin send CMD data
	os_delay_us(12);
	// Send CMD data

	for (i = 0; i < 4; i++) {
		// DCK = 0;
		digitalWrite(_pin_dcki, LOW);
		if (command_data & 0x80) {
			// DI = 1;
			digitalWrite(_pin_di, HIGH);
		} else {
			// DI = 0;
			digitalWrite(_pin_di, LOW);
		}
		// DCK = 1;
		digitalWrite(_pin_dcki, HIGH);
		command_data = command_data << 1;
		if (command_data & 0x80) {
			// DI = 1;
			digitalWrite(_pin_di, HIGH);
		} else {
			// DI = 0;
			digitalWrite(_pin_di, LOW);
		}
		// DCK = 0;
		digitalWrite(_pin_dcki, LOW);
		// DI = 0;
		digitalWrite(_pin_di, LOW);
		command_data = command_data << 1;
	}

	// TStart > 12us. Delay 12 us.
	os_delay_us(12);
	// Send 16 DI pulse，at 14 pulse's falling edge store CMD data, and
	// at 16 pulse's falling edge convert to duty mode.
	_di_pulse(16);
	// TStop > 12us.
	os_delay_us(12);
	// ets_intr_unlock();
}

void my9291::_send() {

	unsigned char i = 0;
	unsigned char channel = 0;
	unsigned char bit_length = 8;
	unsigned int duty_current = 0;

	// Color to show
	unsigned int duty[4] = {0};
	if (_state) {
        duty[0] = _color.red;
        duty[1] = _color.green;
        duty[2] = _color.blue;
        duty[3] = _color.white;
    }

	switch (_command.bit_width) {

		case MY9291_CMD_BIT_WIDTH_16:
			bit_length = 16;
			break;
		case MY9291_CMD_BIT_WIDTH_14:
			bit_length = 14;
			break;
		case MY9291_CMD_BIT_WIDTH_12:
			bit_length = 12;
			break;
		case MY9291_CMD_BIT_WIDTH_8:
			bit_length = 8;
			break;
		default:
			bit_length = 8;
			break;
	}

	// ets_intr_lock();
	// TStop > 12us.
	os_delay_us(12);

	for (channel = 0; channel < 4; channel++) {	//RGBW 4CH
		// RGBW Channel
		duty_current = duty[channel];
		// Send 8bit/12bit/14bit/16bit Data
		for (i = 0; i < bit_length / 2; i++) {
			// DCK = 0;
			digitalWrite(_pin_dcki, LOW);
			if (duty_current & (0x01 << (bit_length - 1))) {
				// DI = 1;
				digitalWrite(_pin_di, HIGH);
			} else {
				// DI = 0;
				digitalWrite(_pin_di, LOW);
			}
			// DCK = 1;
			digitalWrite(_pin_dcki, HIGH);
			duty_current = duty_current << 1;
			if (duty_current & (0x01 << (bit_length - 1))) {
				// DI = 1;
				digitalWrite(_pin_di, HIGH);
			} else {
				// DI = 0;
				digitalWrite(_pin_di, LOW);
			}
			//DCK = 0;
			digitalWrite(_pin_dcki, LOW);
			//DI = 0;
			digitalWrite(_pin_di, LOW);
			duty_current = duty_current << 1;
		}
	}

	// TStart > 12us. Ready for send DI pulse.
	os_delay_us(12);
	// Send 8 DI pulse. After 8 pulse falling edge, store old data.
	_di_pulse(8);
	// TStop > 12us.
	os_delay_us(12);
	// ets_intr_unlock();
}

// -----------------------------------------------------------------------------

my9291_color_t my9291::getColor() {
    return _color;
}

void my9291::setColor(my9291_color_t color) {
    _color.red = color.red;
    _color.green = color.green;
    _color.blue = color.blue;
    _color.white = color.white;
    DEBUG_MSG_MY9291("[MY9291] setColor: %d, %d, %d, %d\n", _color.red, _color.green, _color.blue, _color.white);
    if (_state) _send();
}

bool my9291::getState() {
    return _state;
}

void my9291::setState(bool state) {
    _state = state;
    DEBUG_MSG_MY9291("[MY9291] setState: %s\n", _state ? "true" : "false");
    _send();
}

// -----------------------------------------------------------------------------

my9291::my9291(unsigned char di, unsigned char dcki, my9291_cmd_t command) {

	_pin_di = di;
	_pin_dcki = dcki;

	pinMode(_pin_di, OUTPUT);
	pinMode(_pin_dcki, OUTPUT);

	digitalWrite(_pin_di, LOW);
	digitalWrite(_pin_dcki, LOW);

	// Clear all duty register
	_dcki_pulse(64 / 2);
	_set_cmd(command);

    DEBUG_MSG_MY9291("[MY9291] Initialized\n");

}
