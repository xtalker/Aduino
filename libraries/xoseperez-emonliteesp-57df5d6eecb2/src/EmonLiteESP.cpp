/*

EmonLiteESP 0.1.1

Energy Monitor Library for ESP8266 based on EmonLib
Currently only support current sensing

Copyright (C) 2016 by Xose Pérez <xose dot perez at gmail dot com>

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

#include "Arduino.h"
#include "EmonLiteESP.h"

void EmonLiteESP::initCurrent(current_c callback, unsigned char bits, double ref, double ratio) {

    _currentCallback = callback;
    _adcBits = bits;
    _adcCounts = 1 << bits;
    _referenceVoltage = ref;
    _currentRatio = ratio;
    _currentMidPoint = (_adcCounts>>1);
    calculatePrecision();

};

void EmonLiteESP::warmup() {
    int sample;
    for (unsigned int n = 0; n < WARMUP_COUNTS; n++) {
        sample = _currentCallback();
        _currentMidPoint = (_currentMidPoint + (sample - _currentMidPoint) / _adcCounts);
    }
}

void EmonLiteESP::calculatePrecision() {
    _currentFactor = _currentRatio * _referenceVoltage / _adcCounts;
    _precision = 0;
    _multiplier = 1;
    while (_multiplier * _currentFactor < 1) {
        _multiplier *= 10;
        ++_precision;
    }
    --_precision;
    _multiplier /= 10;
}

void EmonLiteESP::setReference(double ref) {
    _referenceVoltage = ref;
}

void EmonLiteESP::setCurrentRatio(double ratio) {
    _currentRatio = ratio;
}

byte EmonLiteESP::getPrecision() {
    return _precision;
}

void EmonLiteESP::setPrecision(byte precision) {
    _precision = precision;
    _multiplier = 1;
    for (byte i=0; i<_precision; i++) _multiplier *= 10;
}

double EmonLiteESP::getCurrent(unsigned int samples) {

    int sample;
    double filtered;
    double sum;

    for (unsigned int n = 0; n < samples; n++) {

        // Read analog value
        sample = _currentCallback();

        // Digital low pass filter extracts the VDC offset
        _currentMidPoint = (_currentMidPoint + (sample - _currentMidPoint) / _adcCounts);
        filtered = sample - _currentMidPoint;

        // Root-mean-square method
        sum += (filtered * filtered);

    }

    double rms = samples > 0 ? sqrt(sum / samples) : 0;
    double current = _currentFactor * rms;
    current = round(current * _multiplier) / _multiplier;
    return current;

};
