/*
  ==============================================================================

    DiodeClipper.h
    Created: 2 Oct 2025 10:02:56pm
    Author:  dkuzn

  ==============================================================================
*/

#pragma once

#include <cstdint>


class DiodeClipper
{
public:
    // R: series resistance (ohms)
    // Is: diode saturation current (A)
    // nVt: ideality * thermal voltage (V) (typically ~25.85e-3 * n)
    DiodeClipper(float R = 1000.0f, float Is = 1e-9f, float nVt = 0.02585f,
        int maxIter = 50, float tol = 1e-7f);


    // Process one sample (mono). Returns clipped output.
    float process(float input);


    // Setters / getters
    void setSeriesResistance(float R);
    void setSaturationCurrent(float Is);
    void setNVt(float nVt);


private:
    // Solve for the node voltage Vout given Vin using Newton-Raphson
    float solveNode(float Vin) const;


    float R_; // series resistor
    float Is_; // diode saturation current
    float nVt_; // ideality * thermal voltage
    int maxIter_;
    float tol_;
};
