#include "stdafx.h"

class MidiFlasher
{
public:
	MidiFlasher()
	{
		// variable which is both an integer and an array of characters:
		int midiport = 2;       // select which MIDI output port to open
		int flag;           // monitor the status of returning functions

		// Open the MIDI output port
		flag = midiOutOpen(&(this->device), midiport, 0, 0, CALLBACK_NULL);
		if (flag != MMSYSERR_NOERROR) {
			throw "Error opening MIDI Output.";
		}
		this->turnOnInControl();
	}

	~MidiFlasher()
	{
		this->offFrom(0);
		midiOutReset(device);
		midiOutClose(device);
	}

	void SendMidi(float peak) {

		this->turnOnInControl();
		if (peak == 0)
		{
			this->offFrom(0);
		}
		else if (peak < 0.1)
		{
			this->offFrom(1);
		}
		else if (peak < 0.3)
		{
			this->offFrom(2);
		}
		else if (peak < 0.45)
		{
			this->offFrom(3);
		}
		else if (peak < 0.6)
		{
			this->offFrom(4);
		}
		else if (peak < 0.7)
		{
			this->offFrom(5);
		}
		else if (peak < 0.8)
		{
			this->offFrom(6);
		}
		else if (peak < 0.9)
		{
			this->offFrom(7);
		}
		else if (peak < 0.95)
		{
			this->offFrom(8);
		}
		else
		{
			this->offFrom(9);
		}
	}

private:
	int noteStates[256];
	HMIDIOUT device;

	void sendMidiMessage(unsigned char note, int colour)
	{
		if (this->noteStates[note] == colour)
		{
			return;
		}
		this->noteStates[note] = colour;

		union { unsigned long word; unsigned char data[4]; } message;
		message.data[0] = 0x90;  // MIDI note-on message (requires to data bytes)
		message.data[1] = note;    // MIDI note-on message: Key number (60 = middle C)
		message.data[2] = colour;   // MIDI note-on message: Key velocity (100 = loud)
		message.data[3] = 0;     // Unused parameter

		int flag = midiOutShortMsg(this->device, message.word);
		if (flag != MMSYSERR_NOERROR) {
			throw "Warning: MIDI Output is not open.";
		}
	}

	void turnOnInControl()
	{
		union { unsigned long word; unsigned char data[4]; } message;
		message.data[0] = 0x90;  // MIDI note-on message (requires to data bytes)
		message.data[1] = 12;    // MIDI note-on message: Key number (60 = middle C)
		message.data[2] = 127;   // MIDI note-on message: Key velocity (100 = loud)
		message.data[3] = 0;     // Unused parameter

		int flag = midiOutShortMsg(this->device, message.word);
		if (flag != MMSYSERR_NOERROR) {
			throw "Warning: MIDI Output is not open.";
		}
	}

	int getNoteNumber(int note)
	{
		switch (note) {
		case 0:
			return 32;
		case 1:
			return 48;
		case 2:
			return 49;
		case 3:
			return 50;
		case 4:
			return 51;
		case 5:
			return 35;
		case 6:
			return 19;
		case 7:
			return 3;
		default:
			return 1;
		}
	}

	void offFrom(int number)
	{
		for (int i = 0; i < number; ++i) {
			this->sendMidiMessage(96 + i, this->getNoteNumber(i));
			this->sendMidiMessage(112 + i, this->getNoteNumber(i));
		}
		for (int i = number; i < 9; ++i) {
			this->sendMidiMessage(96 + i, 0);
			this->sendMidiMessage(112 + i, 0);
		}
	}
};