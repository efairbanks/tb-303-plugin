#include "Synthesis.h"
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmain"
#include "IPlug_include_in_plug_src.h"
#pragma clang diagnostic pop
#include "IControl.h"
#include "resource.h"

#include <math.h>
#include <algorithm>
#include "blackbox.h"

const int kNumPrograms = 5;

enum EParams
{
  kGain = 0,
  kFreq,
  kNumParams,
  kGainX=79,
  kGainY=62,
  kKnobFrames=128
};

enum ELayout
{
  kWidth = GUI_WIDTH,
  kHeight = GUI_HEIGHT
};

Synthesis::Synthesis(IPlugInstanceInfo instanceInfo)
:   IPLUG_CTOR(kNumParams, kNumPrograms, instanceInfo) {
  TRACE;
  
  IGraphics* pGraphics = MakeGraphics(this, kWidth, kHeight);
  pGraphics->AttachPanelBackground(&COLOR_RED);
  IBitmap knob = pGraphics->LoadIBitmap(KNOB_ID, KNOB_FN, kKnobFrames);
  pGraphics->AttachControl(new IKnobMultiControl(this, kGainX, kGainY, kGain, &knob));
  pGraphics->AttachControl(new IKnobMultiControl(this, 100, 100, kFreq, &knob));
  pGraphics->AttachControl(new ITextControl(this, IRECT(0,0,100,100), new IText(40), "wnags"));
  AttachGraphics(pGraphics);
  CreatePresets();
  threeOhThree=new ThreeOhThree();
}

Synthesis::~Synthesis() {}

void Synthesis::CreatePresets() {
  MakeDefaultPreset((char *) "-", kNumPrograms);
}

void Synthesis::ProcessDoubleReplacing(
                                       double** inputs,
                                       double** outputs,
                                       int nFrames)
{
  // Mutex is already locked for us.
  
  double *leftOutput = outputs[0];
  double *rightOutput = outputs[1];
  
  for (int i = 0; i < nFrames; ++i) {
    mMIDIReceiver.advance();
    int velocity = mMIDIReceiver.getCurrentVelocity();
    if (velocity > 0) {
      threeOhThree->PlayNote(mMIDIReceiver.getLastNoteNumber()*1.0, 0.2, 0.01);
    }
    leftOutput[i] = rightOutput[i] = threeOhThree->Process();
  }
  
  mMIDIReceiver.Flush(nFrames);
}

void Synthesis::Reset()
{
  TRACE;
  IMutexLock lock(this);
}

void Synthesis::OnParamChange(int paramIdx)
{
  IMutexLock lock(this);
}

void Synthesis::ProcessMidiMsg(IMidiMsg* pMsg) {
  mMIDIReceiver.onMessageReceived(pMsg);
}
