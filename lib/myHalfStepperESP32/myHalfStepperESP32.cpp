/***********************************************************************************************
* This file is part of the HalfStepper Arduino library.                                        *
* Copyright © 2016 Thomas J. Biuso III  ALL RIGHTS RESERVED...WHATEVER THAT MEANS.             *
* RELEASED UNDER THE LGPL v3.0 LICENSE; SEE <LICENSE> FILE WITHIN DISTRIBUTION ROOT FOR TERMS. *
************************************************************************************************/

#include "myHalfStepperESP32.h"
using namespace HalfStepperOptions;

/* HalfStepper DEFINITION */

// CONSTRUCTORS
HalfStepper::HalfStepper(dword totalSteps, byte pin1, byte pin2, byte pin3, byte pin4,
	SteppingMode steppingMode, PhasingMode phasingMode, SequenceType sequenceType)
	: Stepper(totalSteps, pin1, pin2, pin3, pin4), _PinCount(4), _TotalSteps(totalSteps),
	_SteppingMode(steppingMode), _PhasingMode(phasingMode), _SequenceType(sequenceType)
{
	if (_SteppingMode == SteppingMode::HALF)
		_TotalSteps *= 2;

	_Pins = new byte[_PinCount];

	_Pins[0] = pin1;
	_Pins[1] = pin2;
	_Pins[2] = pin3;
	_Pins[3] = pin4;

	_Stepper = new Stepper(totalSteps, pin1, pin2, pin3, pin4);

	this->UpdateSteps();
}

// DESTRUCTOR
HalfStepper::~HalfStepper()
{
	delete[] _Steps;
	delete _Stepper;
	delete[] _Pins;
}

void HalfStepper::SetSteppingMode(SteppingMode steppingMode)
{
	_SteppingMode = steppingMode;
	this->UpdateSteps();
}

SteppingMode HalfStepper::GetSteppingMode() const { return _SteppingMode; }

void HalfStepper::SetPhasingMode(PhasingMode phasingMode)
{
	_PhasingMode = phasingMode;
	this->UpdateSteps();
}

PhasingMode HalfStepper::GetPhasingMode() const { return _PhasingMode; }

void HalfStepper::SetSequenceType(SequenceType sequenceType)
{
	_SequenceType = sequenceType;
	this->UpdateSteps();
}

SequenceType HalfStepper::GetSequenceType() const { return _SequenceType; }

void HalfStepper::SetDirection(Direction direction) { _Direction = direction; }

Direction HalfStepper::GetDirection() const { return _Direction; }

void HalfStepper::SetPosition(dword position) { _Position = position; }

dword HalfStepper::GetPosition() const { return _Position; }

int HalfStepper::version()
{
	return 69;	// heh...
}

void HalfStepper::step(int numSteps) { this->Step((long)numSteps); }

void HalfStepper::Step(long numSteps)
{
	if (numSteps == 0)
		return;

	if (numSteps > 0)
	{
		_Direction = Direction::FORWARD;
	}
	else
	{
		_Direction = Direction::REVERSE;
		numSteps = abs(numSteps);
	}

	while (numSteps-- > 0)
	{

		if (_Direction == Direction::FORWARD)
		{
			if (_Position++ == _TotalSteps)
				_Position = 0;
		}
		else
		{
			if (_Position-- == 0)
				_Position = _TotalSteps;
		}

		this->DoStep(_Position % (_PinCount * 2));
	}
}

void HalfStepper::StepForward(dword numSteps) { this->Step((long)numSteps); }

void HalfStepper::StepBackward(dword numSteps) { this->Step((long)-1 * (long)numSteps); }

void HalfStepper::StepTo(dword position)
{
	long numSteps = position - _Position;

	if (position >= _TotalSteps)
		position = position % _TotalSteps;

	if (abs(numSteps) > _TotalSteps / 2)
		numSteps = _Position - position;

	this->step(numSteps);
}

// STEP SEQUENCE RETRIEVAL METHOD
void HalfStepper::UpdateSteps()
{
	if (_Steps != NULL)
		delete[] _Steps;

	if (_PinCount == 2)
	{
		_Steps = new byte[4];

		for (int i = 0; i < 4; i++)
			_Steps[i] = pgm_read_byte_near(&_STEP_SEQUENCES_TWO_PIN[BOOL_TO_INDEX((bool)_SteppingMode)][i]);
	}
	else
	{
		_Steps = new byte[8];

		for (int i = 0; i < 8; i++)
		{
			_Steps[i] = pgm_read_byte_near(&_STEP_SEQUENCES_FOUR_PIN[BOOL_TO_INDEX((bool)_SteppingMode)]
				[BOOL_TO_INDEX((bool)_PhasingMode)][BOOL_TO_INDEX((bool)_SequenceType)][i]);
		}
	}
}

// STEP EXECUTION METHOD
void HalfStepper::DoStep(byte stepIdx)
{
	if (_PinCount == 4)
	{
		digitalWrite(_Pins[0], _Steps[stepIdx] & B1000 ? HIGH : LOW);
		digitalWrite(_Pins[1], _Steps[stepIdx] & B0100 ? HIGH : LOW);
		digitalWrite(_Pins[2], _Steps[stepIdx] & B0010 ? HIGH : LOW);
		digitalWrite(_Pins[3], _Steps[stepIdx] & B0001 ? HIGH : LOW);
	}
	else
	{
		digitalWrite(_Pins[0], _Steps[stepIdx] & B0010 ? HIGH : LOW);
		digitalWrite(_Pins[1], _Steps[stepIdx] & B0001 ? HIGH : LOW);
	}
}

