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

	delayMicroseconds(1000);

	// Charge carry
	digitalWrite(cathodePin, HIGH);
	delayMicroseconds(100);

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
	if(cap > 50){
		// pin  9 --> 1.2c: R0 1.2d: C3
		// pin 15 --> 1.2c: C1 1.2d: R5
		cap = measureDiodeCapacitance(9, 15);
		if(cap > 50){
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

void Dots::update(void)
{
    uint8_t data;

	outp(_rowPins[_row], !_anodeCommon);
	_row++;
	if(_row >= _numOfRows){
		_row = 0;
#if DOTS_DOUBLE_BUFFERING
		if(_flip){
			_frontBuffer[0] = _buffer[0];
			_frontBuffer[1] = _buffer[1];
			_frontBuffer[2] = _buffer[2];
			_frontBuffer[3] = _buffer[3];
			_frontBuffer[4] = _buffer[4];
			_frontBuffer[5] = _buffer[5];
			_frontBuffer[6] = _buffer[6];
			_frontBuffer[7] = _buffer[7];
			_flip = false;
		}
#endif
	}
#if DOTS_DOUBLE_BUFFERING
	data = _frontBuffer[_row];
#else
	data = _buffer[_row];
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
}
#else
void Dots::update(void)
{
	uint8_t i,data,mask;
	digitalWrite(_rowPins[_row], !_anodeCommon);
	_row++;
	if(_row >= _numOfRows){
		_row = 0;
#if DOTS_DOUBLE_BUFFERING
		if(_flip){
			for(i=0;i<8;i++){
				_frontBuffer[i] = _buffer[i];
			}
			_flip = false;
		}
#endif
	}
#if DOTS_DOUBLE_BUFFERING
	data = _frontBuffer[_row];
#else
	data = _buffer[_row];
#endif
	data = data ^ _anodeCommon;
	mask = 0x80;
	for(i=0;i<_numOfCols;i++){
		digitalWrite(_colPins[i], data & mask);
		mask >>= 1;
	}
	digitalWrite(_rowPins[_row], _anodeCommon);
}
#endif

#if DOTS_DOUBLE_BUFFERING
void Dots::flip(void)
{
	_flip = true;
	while(_flip);
}
#endif

Dots *Dots::active_object = 0;

SIGNAL(TIMER0_COMPA_vect)
{
	if(Dots::active_object){
		Dots::active_object->update();
	}
}
