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
    void updateDCBlocker();
    float toneFilter(float x);
    float DC_blocker(float x);

    float inputGain;
    float bias;
    float outputGain;
    float toneCutoff; // Hz
    float sampleRate;

    // LPF 1 order
    float a0 = 0.0f;
    float b1 = 0.0f;
    float z1 = 0.0f;

    // DC blocker
    float dc_x1 = 0.0f;
    float dc_y1 = 0.0f;
    float dc_R = 0.0f;
};
