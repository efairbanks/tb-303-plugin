//
//  MIDIReceiver.cpp
//  Synthesis
//
//  Created by Martin on 08.04.14.
//
//

#include "MIDIReceiver.h"

void MIDIReceiver::onMessageReceived(IMidiMsg* midiMessage) {
    IMidiMsg::EStatusMsg status = midiMessage->StatusMsg();
    // We're only interested in Note On/Off messages (not CC, pitch, etc.)
    if(status == IMidiMsg::kNoteOn || status == IMidiMsg::kNoteOff) {
        mMidiQueue.Add(midiMessage);
    }
}

void MIDIReceiver::advance() {
    if(mMidiQueue.Empty()) mCurrentVelocity=0;
    while (!mMidiQueue.Empty()) {
        IMidiMsg* midiMessage = mMidiQueue.Peek();
        if (midiMessage->mOffset > mOffset) {
            mCurrentVelocity=0;
            break;
        }
        
        IMidiMsg::EStatusMsg status = midiMessage->StatusMsg();
        int noteNumber = midiMessage->NoteNumber();
        int velocity = midiMessage->Velocity();
        // There are only note on/off messages in the queue, see ::OnMessageReceived
        if(status == IMidiMsg::kNoteOn) {
            mCurrentVelocity = velocity;
            mLastNoteNumber = noteNumber;
            mLastFrequency = noteNumberToFrequency(mLastNoteNumber);
        }
        /*
        if (status == IMidiMsg::kNoteOn && velocity) {
            if(mKeyStatus[noteNumber] == false) {
                mKeyStatus[noteNumber] = true;
                mNumKeys += 1;
            }
            // A key pressed later overrides any previously pressed key:
            if (noteNumber != mLastNoteNumber) {
                mLastNoteNumber = noteNumber;
                mLastFrequency = noteNumberToFrequency(mLastNoteNumber);
                mLastVelocity = velocity;
            }
        } else {
            if(mKeyStatus[noteNumber] == true) {
                mKeyStatus[noteNumber] = false;
                mNumKeys -= 1;
            }
            // If the last note was released, nothing should play:
            if (noteNumber == mLastNoteNumber) {
                mLastNoteNumber = -1;
                mLastFrequency = -1;
                mLastVelocity = 0;
            }
        }*/
        mMidiQueue.Remove();
    }
    mOffset++;
}