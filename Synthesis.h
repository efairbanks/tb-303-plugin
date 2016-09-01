#ifndef __SYNTHESIS__
#define __SYNTHESIS__

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wextra-tokens"
#include "IPlug_include_in_plug_hdr.h"
#pragma clang diagnostic pop

#include "Oscillator.h"
#include "MIDIReceiver.h"
#include "blackbox.h"

class Synthesis : public IPlug
{
public:
  Synthesis(IPlugInstanceInfo instanceInfo);
  ~Synthesis();

  void Reset();
  void OnParamChange(int paramIdx);
  void ProcessDoubleReplacing(double** inputs, double** outputs, int nFrames);
  // to receive MIDI messages:
  void ProcessMidiMsg(IMidiMsg* pMsg);
  
private:
  void CreatePresets();
  MIDIReceiver mMIDIReceiver;
  ThreeOhThree* threeOhThree;
};

#endif
