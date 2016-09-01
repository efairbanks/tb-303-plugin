//
//  blackbox.h
//  Synthesis
//
//  Created by Eric Fairbanks on 7/8/15.
//
//

#ifndef MyFirstPlugin_blackbox_h
#define MyFirstPlugin_blackbox_h

#include "stdio.h"
#include "stdlib.h"
#include "math.h"

class BlackBox {
protected:
    double sampleRate;
    BlackBox(double sampleRate) {
        this->sampleRate=sampleRate;
    }
public:
    virtual double Process() = 0;
    //virtual void SetSampleRate(double sampleRate) {
    //   this->sampleRate = sampleRate;
    //}
};
class SinOsc : BlackBox {
protected:
    double frequency;
    double amplitude;
    double delta;
public:
    SinOsc(double frequency, double amplitude, double sampleRate) : BlackBox(sampleRate) {
        this->frequency=frequency;
        this->amplitude=amplitude;
    }
    double Process() {
        double output = sin(this->delta*2*M_PI);
        this->delta = this->delta + (this->frequency / this->sampleRate);
        return output * this->amplitude;
    }
    double Process(double phase) {
        double output = sin(this->delta*2*M_PI+phase);
        this->delta = this->delta + (this->frequency / this->sampleRate);
        return output * this->amplitude;
    }
    void SetAmplitude(double amplitude) { this->amplitude = amplitude; }
    void SetFrequency(double frequency) { this->frequency = frequency; }
};

class Pulse : BlackBox {
protected:
    double frequency;
    double amplitude;
    double pulseWidth;
    double delta;
public:
    Pulse(double frequency, double amplitude, double pulseWidth, double sampleRate) : BlackBox(sampleRate)
    {
        this->frequency=frequency;
        this->amplitude=amplitude;
        this->pulseWidth=pulseWidth;
        this->delta=0;
    }
    double Process()
    {
        double output=0;
        if(this->delta+(this->frequency/this->sampleRate) < this->pulseWidth){
            output=this->amplitude;
        } else {
            output=-this->amplitude;
        }
        this->delta = this->delta+(this->frequency/48000);
        while(this->delta>1) this->delta=this->delta - 1;
        return output;
    }
    void SetAmplitude(double amplitude) { this->amplitude = amplitude; }
    void SetFrequency(double frequency) { this->frequency = frequency; }
};

class Saw : BlackBox
{
protected:
    double phase = 0;
    double delta = 0;
    double amplitude = 1;
public:
    Saw(double frequency, double amplitude, double sampleRate) : BlackBox(sampleRate) {
        this->SetAmplitude(amplitude);
        this->SetFrequency(frequency);
    }
    double Process() {
        this->phase = this->phase + this->delta;
        double output = ((this->phase*2)-1)*this->amplitude;
        while(this->phase>1) this->phase = this->phase - 1;
        return output;
    }
    void SetAmplitude(double amplitude) { this->amplitude = amplitude; }
    void SetFrequency(double frequency) { this->delta = (frequency/this->sampleRate); }
};
class Tri : BlackBox
{
protected:
    double frequency;
    double amplitude;
    double delta;
public:
    Tri(double frequency, double amplitude, double sampleRate) : BlackBox(sampleRate) {
        this->frequency=frequency;
        this->amplitude=amplitude;
    }
    double Process()
    {
        double output = 0;
        if( this->delta < 0.25 ){
            output = delta*4;
        } else if( this->delta < 0.5 ){
            output = 1-((delta-0.25)*4);
        } else if( this->delta < 0.75 ){
            output = -((delta-0.5)*4);
        } else if( this->delta < 1 ){
            output = -(1-((delta-0.75)*4));
        } else {
            output = 0;
        }
        output = output*this->amplitude;
        this->delta+=(this->frequency/this->sampleRate);
        while(this->delta>1) this->delta = this->delta - 1;
        return output;
    }
    void SetAmplitude(double amplitude) { this->amplitude = amplitude; }
    void SetFrequency(double frequency) { this->frequency = frequency; }
};
class WhiteNoise : BlackBox {
protected:
    int quantize;
public:
    WhiteNoise(int quantize, double sampleRate) : BlackBox(sampleRate) {
        this->quantize=quantize;
    }
    double Process() {
        return 1-(rand()%this->quantize)/((double)quantize)*2;
    }
};
class Lag : BlackBox
{
protected:
    bool initialized = false;
    double value;
public:
    Lag(double sampleRate) : BlackBox(sampleRate) { }
    double Process(double target, double rate) {
        if(initialized == false) {
            initialized = true;
            value = target;
        }
        if (rate <= 0) {
            value = target;
        } else {
            value += (target - value) / (rate * sampleRate);
        }
        return value;
    }
    double Process() {
        return value;
    }
    double SetValue(double target) {
        value = target;
        return value;
    }
};
typedef enum {
    LowPass,
    HighPass,
    BandPass,
    Notch,
    Peak,
    LowShelf,
    HighShelf
} FilterType;
class Biquad : BlackBox {
public:
    Biquad(double sampleRate) : BlackBox(sampleRate)
    {
        type = LowPass;
        a0 = 1.0;
        a1 = a2 = b1 = b2 = 0.0;
        Fc = 0.50;
        Q = 0.707;
        peakGain = 0.0;
        z1 = z2 = 0.0;
    }
    Biquad(FilterType type, double frequency, double Q, double peakGainDB, double sampleRate) : BlackBox(sampleRate)
    {
        SetBiquad(type, frequency, Q, peakGainDB);
        z1 = z2 = 0.0;
    }
    void SetType(FilterType type)
    {
        this->type = type;
        calcBiquad();
    }
    void SetQ(double Q)
    {
        this->Q = Q;
        calcBiquad();
    }
    void SetFc(double Fc)
    {
        this->Fc = Fc > 0.5 ? 0.5 : Fc;
        calcBiquad();
    }
    void SetFrequency(double frequency) {
        SetFc(frequency/this->sampleRate);
    }
    void SetPeakGain(double peakGainDB)
    {
        this->peakGain = peakGainDB;
        calcBiquad();
    }
    void SetBiquad(FilterType type, double frequency, double Q, double peakGainDB)
    {
        double Fc = frequency/this->sampleRate;
        this->Fc = Fc > 0.5 ? 0.5 : Fc;
        this->type = type;
        this->Q = Q;
        SetPeakGain(peakGainDB);
    }
    double Process(double input) {
        double output = input * a0 + z1;
        z1 = input * a1 + z2 - b1 * output;
        z2 = input * a2 - b2 * output;
        return output;
    }
    double Process() {
        double output = 0 * a0 + z1;
        z1 = 0 * a1 + z2 - b1 * output;
        z2 = 0 * a2 - b2 * output;
        return output;
    }
    void calcBiquad()
    {
        double norm;
        double V = pow(10, abs(peakGain) / 20.0);
        double K = tan(M_PI * Fc);
        switch (this->type) {
            case LowPass:
                norm = 1 / (1 + K / Q + K * K);
                a0 = K * K * norm;
                a1 = 2 * a0;
                a2 = a0;
                b1 = 2 * (K * K - 1) * norm;
                b2 = (1 - K / Q + K * K) * norm;
                break;
                
            case HighPass:
                norm = 1 / (1 + K / Q + K * K);
                a0 = 1 * norm;
                a1 = -2 * a0;
                a2 = a0;
                b1 = 2 * (K * K - 1) * norm;
                b2 = (1 - K / Q + K * K) * norm;
                break;
                
            case BandPass:
                norm = 1 / (1 + K / Q + K * K);
                a0 = K / Q * norm;
                a1 = 0;
                a2 = -a0;
                b1 = 2 * (K * K - 1) * norm;
                b2 = (1 - K / Q + K * K) * norm;
                break;
                
            case Notch:
                norm = 1 / (1 + K / Q + K * K);
                a0 = (1 + K * K) * norm;
                a1 = 2 * (K * K - 1) * norm;
                a2 = a0;
                b1 = a1;
                b2 = (1 - K / Q + K * K) * norm;
                break;
                
            case Peak:
                if (peakGain >= 0) {    // boost
                    norm = 1 / (1 + 1/Q * K + K * K);
                    a0 = (1 + V/Q * K + K * K) * norm;
                    a1 = 2 * (K * K - 1) * norm;
                    a2 = (1 - V/Q * K + K * K) * norm;
                    b1 = a1;
                    b2 = (1 - 1/Q * K + K * K) * norm;
                }
                else {    // cut
                    norm = 1 / (1 + V/Q * K + K * K);
                    a0 = (1 + 1/Q * K + K * K) * norm;
                    a1 = 2 * (K * K - 1) * norm;
                    a2 = (1 - 1/Q * K + K * K) * norm;
                    b1 = a1;
                    b2 = (1 - V/Q * K + K * K) * norm;
                }
                break;
            case LowShelf:
                if (peakGain >= 0) {    // boost
                    norm = 1 / (1 + sqrt(2) * K + K * K);
                    a0 = (1 + sqrt(2*V) * K + V * K * K) * norm;
                    a1 = 2 * (V * K * K - 1) * norm;
                    a2 = (1 - sqrt(2*V) * K + V * K * K) * norm;
                    b1 = 2 * (K * K - 1) * norm;
                    b2 = (1 - sqrt(2) * K + K * K) * norm;
                }
                else {    // cut
                    norm = 1 / (1 + sqrt(2*V) * K + V * K * K);
                    a0 = (1 + sqrt(2) * K + K * K) * norm;
                    a1 = 2 * (K * K - 1) * norm;
                    a2 = (1 - sqrt(2) * K + K * K) * norm;
                    b1 = 2 * (V * K * K - 1) * norm;
                    b2 = (1 - sqrt(2*V) * K + V * K * K) * norm;
                }
                break;
            case HighShelf:
                if (peakGain >= 0) {    // boost
                    norm = 1 / (1 + sqrt(2) * K + K * K);
                    a0 = (V + sqrt(2*V) * K + K * K) * norm;
                    a1 = 2 * (K * K - V) * norm;
                    a2 = (V - sqrt(2*V) * K + K * K) * norm;
                    b1 = 2 * (K * K - 1) * norm;
                    b2 = (1 - sqrt(2) * K + K * K) * norm;
                }
                else {    // cut
                    norm = 1 / (V + sqrt(2*V) * K + K * K);
                    a0 = (1 + sqrt(2) * K + K * K) * norm;
                    a1 = 2 * (K * K - 1) * norm;
                    a2 = (1 - sqrt(2) * K + K * K) * norm;
                    b1 = 2 * (K * K - V) * norm;
                    b2 = (V - sqrt(2*V) * K + K * K) * norm;
                }
                break;
        }
        return;
    }
private:
    FilterType type;
    double a0, a1, a2, b1, b2;
    double Fc, Q, peakGain;
    double z1, z2;
};
class Line {
public:
    double from;
    double to;
    double duration;
    
    double delta;
    
    Line(double from, double to, double duration)
    {
        this->from = from;
        this->to = to;
        this->duration = duration;
        this->delta = -1;
    }
    
    double Process()
    {
        double output = 0;
        if(this->delta > -1)
        {
            output = ((this->to*this->delta)/this->duration) + ((this->from*(this->duration-this->delta))/this->duration);
            this->delta  = this->delta + (1.0/48000.0);
            if(this->delta>this->duration + (1.0/48000.0)) this->delta = -1;
        } else {
            output = this->to;
        }
        return output;
    }
    
    void Reset()
    {
        this->delta = 0;
    }
};
class ThreeOhThree
{
public:
    Pulse* pulse = new Pulse (440, 1, 0.5, 48000);
    Saw* saw = new Saw(440, 1, 48000);
    class Line* outEnv = new class Line (1, 0, 0.3);
    class Line* inEnv = new class Line (0, 1, 0.01);
    Biquad* lowpass = new Biquad (LowPass, 440, 4, 0, 48000);
    Lag* portamento = new Lag(48000);
    
    double note = 0;
    double lag = 0.01;
    
    double Process()
    {
        double outEnvValue = outEnv->Process ();
        double inEnvValue = inEnv->Process ();
        double freq = pow (2, portamento->Process(note, lag) / 12) * 25.5;
        pulse->SetFrequency(freq);
        saw->SetFrequency(freq);
        lowpass->SetFc ((freq * 8 * outEnvValue * inEnvValue + 20.0) / 48000.0);
        return tanh(lowpass->Process((pulse->Process () + saw->Process()) * outEnvValue * inEnvValue) * 4);
    }
    
    void PlayNote(double midiNote)
    {
        note = midiNote;
        inEnv->Reset ();
        outEnv->Reset ();
    }
    
    void PlayNote(double midiNote, double duration, double lag)
    {
        outEnv->duration = duration;
        this->lag = lag;
        note = midiNote;
        inEnv->Reset ();
        outEnv->Reset ();
    }
};
#endif
