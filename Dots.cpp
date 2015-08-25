/*
 Dots.cpp - Arduino library for 8x8/5x7 Dot LED Matrix.
 Copyright 2010,2011 arms22. All right reserved.
 
 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "Dots.h"
#include <avr/interrupt.h>

#define ANODE_COMMON (0xff)
#define CATHODE_COMMON (0x00)

Dots::Dots(int which)
{
	if(which == Dots_12d){
		init12d();
	}
	else if(which == Dots_12c){
		init12c(ANODE_COMMON);
	}
	else if(which == Dots_12cc){
		init12c(CATHODE_COMMON);
	}
	else{
		_autoDetect = true;
	}
}

Dots::Dots(uint8_t r0,uint8_t r1,uint8_t r2,uint8_t r3,
		   uint8_t r4,uint8_t r5,uint8_t r6,uint8_t r7,
		   uint8_t c0,uint8_t c1,uint8_t c2,uint8_t c3,
		   uint8_t c4,uint8_t c5,uint8_t c6,uint8_t c7)
{
	_rowPins[0] = r0; _rowPins[1] = r1; _rowPins[2] = r2; _rowPins[3] = r3;
	_rowPins[4] = r4; _rowPins[5] = r5; _rowPins[6] = r6; _rowPins[7] = r7;
	
	_colPins[0] = c0; _colPins[1] = c1; _colPins[2] = c2; _colPins[3] = c3;
	_colPins[4] = c4; _colPins[5] = c5; _colPins[6] = c6; _colPins[7] = c7;
	
	_numOfRows = 8;
	_numOfCols = 8;
	_autoDetect = false;
	_anodeCommon = ANODE_COMMON;
}

Dots::Dots(uint8_t r0,uint8_t r1,uint8_t r2,uint8_t r3,
		   uint8_t r4,uint8_t r5,uint8_t r6,
		   uint8_t c0,uint8_t c1,uint8_t c2,uint8_t c3,
		   uint8_t c4)
{
	_rowPins[0] = r0; _rowPins[1] = r1; _rowPins[2] = r2; _rowPins[3] = r3;
	_rowPins[4] = r4; _rowPins[5] = r5; _rowPins[6] = r6;
	
	_colPins[0] = c0; _colPins[1] = c1; _colPins[2] = c2; _colPins[3] = c3;
	_colPins[4] = c4;
	
	_numOfRows = 7;
	_numOfCols = 5;
	_autoDetect = false;
	_anodeCommon = ANODE_COMMON;
}

void Dots::init12d(void)
{
	_rowPins[0] = 2;	_rowPins[1] = 3;	_rowPins[2] = 4;	_rowPins[3] = 13;
	_rowPins[4] = 14;   _rowPins[5] = 15;   _rowPins[6] = 16;   _rowPins[7] = 17;
	
	_colPins[0] = 12;   _colPins[1] = 11;   _colPins[2] = 10;   _colPins[3] = 9;
	_colPins[4] = 5;	_colPins[5] = 6;	_colPins[6] = 7;	_colPins[7] = 8;
	
	_numOfRows = 8;
	_numOfCols = 8;
	_autoDetect = false;
	_anodeCommon = ANODE_COMMON;
}

void Dots::init12c(uint8_t common)
{
	_rowPins[0] = 9;	_rowPins[1] = 4;	_rowPins[2] = 10;   _rowPins[3] = 6;
	_rowPins[4] = 17;   _rowPins[5] = 11;   _rowPins[6] = 16;   _rowPins[7] = 13;
	
	_colPins[0] = 5;	_colPins[1] = 15;   _colPins[2] = 14;   _colPins[3] = 8;
	_colPins[4] = 12;   _colPins[5] = 7;	_colPins[6] = 3;	_colPins[7] = 2;
	
	_numOfRows = 8;
	_numOfCols = 8;
	_autoDetect = false;
	_anodeCommon = common;
}

static uint8_t measureDiodeCapacitance(uint8_t anodePin, uint8_t cathodePin)
{
	uint8_t result;

	// Discharge carry
	pinMode(anodePin, OUTPUT);
	pinMode(cathodePin, OUTPUT);

	digitalWrite(anodePin, LOW);
	digitalWrite(cathodePin, LOW);

	delayMicroseconds(100);

	// Charge carry
	digitalWrite(cathodePin, HIGH);
	delayMicroseconds(10);

	// Measure Capacitance
	pinMode(cathodePin, INPUT);
	for(result=0; digitalRead(cathodePin) && (result < 255); result++)
		;

	pinMode(anodePin, INPUT);
	return result;
}

void Dots::autoDetect(void)
{
	uint8_t i, cap;
	for(i=2; i<18; i++){
		pinMode(i, INPUT);
		digitalWrite(i, LOW);
	}
	// pin 16 --> 1.2c: R7 1.2d: R7
	// pin 12 --> 1.2c: C5 1.2d: C1
	cap = measureDiodeCapacitance(16, 12);
	if(cap > 128){
		// pin  9 --> 1.2c: R0 1.2d: C3
		// pin 15 --> 1.2c: C1 1.2d: R5
		cap = measureDiodeCapacitance(9, 15);
		if(cap > 128){
			init12c(ANODE_COMMON);
		}else{
			init12d();
		}
	}else{
		init12c(CATHODE_COMMON);
	}
}

void Dots::begin(void)
{
	uint8_t i;
	if(_autoDetect){
		autoDetect();
	}
	for(i=0;i<_numOfRows;i++){
		pinMode(_rowPins[i], OUTPUT);
	}
	for(i=0;i<_numOfCols;i++){
		pinMode(_colPins[i], OUTPUT);
	}
	clear();
	Dots::active_object = this;
	_row = 0;
	TCCR0A	= 0;
	OCR0A	= 0;
	TIFR0  |= _BV(OCF0A);
	TIMSK0 |= _BV(OCIE0A);
}

void Dots::end(void)
{
	TIMSK0 &= ~_BV(OCIE0A);
	Dots::active_object = 0;
}

void Dots::write(uint8_t x, uint8_t y, int value)
{
	uint8_t tmp,msk;
	tmp = _buffer[y];
	msk = 0x80 >> x;
	tmp = tmp & ~msk;
	tmp = tmp | (value ? msk : 0);
	_buffer[y] = tmp;
}

void Dots::write(uint8_t y, int value)
{
	_buffer[y] = value;
}

void Dots::write(uint8_t y, const uint8_t *buffer, size_t size)
{
	for(;size;size--)
		_buffer[(y++) & 0x07] = *buffer++;
}

void Dots::clear(void)
{
	uint8_t i;
	for(i=0;i<8;i++){
		_buffer[i] = 0;
	}
}

#if DOTS_FAST_WRITE_ENABLE
static inline void outp(uint8_t pin, uint8_t val)
{
	uint8_t bit = digitalPinToBitMask(pin);
	uint8_t port = digitalPinToPort(pin);
	volatile uint8_t *out = portOutputRegister(port);
	if (val == LOW) {
		*out &= ~bit;
	} else {
		*out |= bit;
	}
}

#if DOTS_ADJUST_BRIGHTNESS
static inline uint8_t popcount(uint8_t data)
{
	data = (data & 0x55) + ((data & 0xaa) >> 1);
	data = (data & 0x33) + ((data & 0xcc) >> 2);
	data = (data & 0x0f) + ((data & 0xf0) >> 4);
	return data;
}
#endif

void Dots::update(void)
{
    uint8_t data;

#if DOTS_ADJUST_BRIGHTNESS
    uint8_t adjust = 0;
#endif
	outp(_rowPins[_row], !_anodeCommon);
	_row++;
	if(_row >= _numOfRows){
		_row = 0;
	}
	data = _buffer[_row];
#if DOTS_ADJUST_BRIGHTNESS
	adjust = popcount(data);
	adjust = (adjust << 4) + (adjust << 3);
#endif
	data = data ^ _anodeCommon;
    switch (_numOfCols) {
        case 8: outp(_colPins[7], (data & 0x01));
        case 7: outp(_colPins[6], (data & 0x02));
        case 6: outp(_colPins[5], (data & 0x04));
        case 5: outp(_colPins[4], (data & 0x08));
        case 4: outp(_colPins[3], (data & 0x10));
        case 3: outp(_colPins[2], (data & 0x20));
        case 2: outp(_colPins[1], (data & 0x40));
        case 1: outp(_colPins[0], (data & 0x80));
    }
	outp(_rowPins[_row], _anodeCommon);

#define UPDATE_INTERVAL 64
    OCR0A += UPDATE_INTERVAL;
#if DOTS_ADJUST_BRIGHTNESS
	OCR0A += adjust;
#endif
}
#else
void Dots::update(void)
{
	uint8_t i,data,mask;
	digitalWrite(_rowPins[_row], !_anodeCommon);
	_row++;
	if(_row >= _numOfRows){
		_row = 0;
#if DOTS_CAP_SENSE_ENABLE
		scanRow();
#endif
	}
	data = _buffer[_row];
	data = data ^ _anodeCommon;
	mask = 0x80;
	for(i=0;i<_numOfCols;i++){
		digitalWrite(_colPins[i], data & mask);
		mask >>= 1;
	}
	digitalWrite(_rowPins[_row], _anodeCommon);
}
#endif

#if DOTS_CAP_SENSE_ENABLE
uint8_t Dots::capacityAt(uint8_t x, uint8_t y)
{
	return _capacities[y][x];
}

void Dots::scanRow(void)
{
	uint8_t i, cnt;
	uint8_t bits[_numOfCols];
	uint8_t ports[_numOfCols];
	
	// All Cathodes Output High.
	for(i=0; i<_numOfCols; i++){
		bits[i] = digitalPinToBitMask(_colPins[i]);
		ports[i] = digitalPinToPort(_colPins[i]);
        
		*portModeRegister(ports[i]) |= bits[i];
		*portOutputRegister(ports[i]) |= bits[i];
        
		_capacities[_scanRow][i] = DOTS_CAP_SENSE_MEASURE_LIMITS;
	}
	
	// Anode Outout Low.
	pinMode(_rowPins[_scanRow], OUTPUT);
	
	// Wait for a while to Charge
	delayMicroseconds(10);
	
	// Isolate the pin connected to cathods, turn off internal pull-up resistor
	for(i=0; i<_numOfCols; i++){
		*portModeRegister(ports[i]) &= ~bits[i];
		*portOutputRegister(ports[i]) &= ~bits[i];
	}
	
	// Measure How long it takes for Cathodes to become Low
	for(cnt=0; cnt<DOTS_CAP_SENSE_MEASURE_LIMITS; cnt++){
		for(i=0; i<_numOfCols; i++){
			if((cnt < _capacities[_scanRow][i]) && ((*portInputRegister(ports[i]) & bits[i]) == 0)){
				_capacities[_scanRow][i] = cnt;
			}
		}
	}
	
	pinMode(_rowPins[_scanRow], INPUT);
	
	// All Cathodes Output
	for(i=0; i<_numOfCols; i++){
		*portModeRegister(ports[i]) |= bits[i];
	}
    
	_scanRow++;
	if(_scanRow >= _numOfRows){
		_scanRow = 0;
	}
}
#endif

Dots *Dots::active_object = 0;

SIGNAL(TIMER0_COMPA_vect)
{
	if(Dots::active_object){
		Dots::active_object->update();
	}
}
