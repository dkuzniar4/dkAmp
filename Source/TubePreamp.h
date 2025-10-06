#pragma once
#include <cmath>

class TubePreamp
{
public:
    TubePreamp();

    void setSampleRate(float sr);
    void setGain(float g);
    void setBias(float b);
    void setOutput(float o);
    void setTone(float cutoffHz);

    float process(float input);

private:
    void updateToneFilter();
    float toneFilter(float x);

    float inputGain;
    float bias;
    float outputGain;
    float toneCutoff; // Hz
    float sampleRate;

    // LPF 1 order
    float a0, b1, z1;
};
