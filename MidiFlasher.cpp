#include "stdafx.h"

class MidiFlasher
{
public:
	MidiFlasher(int midiport)
	{
		int numDevices = midiOutGetNumDevs();
		if (midiport >= numDevices)
		{
			throw L"Invalid device number.";
		}

		int flag = midiOutOpen(&(this->device), midiport, 0, 0, CALLBACK_NULL);
		if (flag != MMSYSERR_NOERROR) {
			throw L"Error opening MIDI Output.";
		}
	}

	~MidiFlasher()
	{
		try
		{
			this->offFrom(0);
			midiOutReset(device);
			midiOutClose(device);
		}
		catch (const LPCWSTR msg)
		{
			// ignore
		}
	}

	void Debug(const WCHAR* szFormat, ...)
	{
		WCHAR szBuff[1024];
		va_list arg;
		va_start(arg, szFormat);
		_vsnwprintf_s(szBuff, sizeof(szBuff), szFormat, arg);
		va_end(arg);

		OutputDebugString(szBuff);
	}

	void SendMidi(float peak) {

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
	int noteStates[256] = {};
	HMIDIOUT device;

	void sendMidiMessage(unsigned char note, int colour)
	{
		// dont send a signal if it is already set correctly
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
			throw L"MIDI Output is not open.";
		}
	}

	int getNoteColour(int note)
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
			this->sendMidiMessage(96 + i, this->getNoteColour(i));
			this->sendMidiMessage(112 + i, this->getNoteColour(i));
		}
		for (int i = number; i < 9; ++i) {
			this->sendMidiMessage(96 + i, 0);
			this->sendMidiMessage(112 + i, 0);
		}
	}
};