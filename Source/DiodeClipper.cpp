/*
  ==============================================================================

    DiodeClipper.cpp
    Created: 2 Oct 2025 10:02:56pm
    Author:  dkuzn

  ==============================================================================
*/

#include "DiodeClipper.h"
#include <cmath>
#include <algorithm>

DiodeClipper::DiodeClipper(float R, float Is, float nVt, int maxIter, float tol)
    : R_(R), Is_(Is), nVt_(nVt), maxIter_(maxIter), tol_(tol)
{
}

void DiodeClipper::setSeriesResistance(float R) { R_ = R; }
void DiodeClipper::setSaturationCurrent(float Is) { Is_ = Is; }
void DiodeClipper::setNVt(float nVt) { nVt_ = nVt; }

float DiodeClipper::process(float input)
{
    return solveNode(input);
}

// We model two anti-parallel diodes between the node and ground.
// The diode current (sum of both directions) can be written as:
// Id(V) = 2 * Is * sinh(V / nVt)
// Circuit equation: (Vin - Vout) / R = Id(Vout)
// Solve f(Vout) = (Vin - Vout)/R - 2*Is*sinh(Vout/nVt) = 0

float DiodeClipper::solveNode(float Vin) const
{
    // Quick path: if R is very large (no clipping) just return Vin
    if (R_ <= 0.0f || Is_ <= 0.0f || nVt_ <= 0.0f)
        return Vin;

    // Initial guess: clamp to Vin but keep reasonable
    float V = std::clamp(Vin, -1.0f, 1.0f);

    for (int i = 0; i < maxIter_; ++i)
    {
        // Id = 2*Is*sinh(V/nVt)
        float x = V / nVt_;
        // For large x, sinh/ cosh can overflow; use safe checks
        float sinh_x;
        float cosh_x;
        if (std::fabs(x) < 20.0f)
        {
            sinh_x = std::sinh(x);
            cosh_x = std::cosh(x);
        }
        else
        {
            // For very large |x|, sinh ~ 0.5*exp(|x|)*sign(x), cosh ~ 0.5*exp(|x|)
            float ex = std::exp(std::fabs(x));
            sinh_x = (x >= 0.0f) ? 0.5f * ex : -0.5f * ex;
            cosh_x = 0.5f * ex;
        }

        float Id = 2.0f * Is_ * sinh_x;

        // f(V) = (Vin - V)/R - Id
        float f = (Vin - V) / R_ - Id;

        // f'(V) = -1/R - 2*Is*(1/nVt)*cosh(V/nVt)
        float df = -1.0f / R_ - 2.0f * Is_ * (cosh_x / nVt_);

        // Newton step
        float dV = 0.0f;
        if (df != 0.0f)
            dV = -f / df;

        // Damping the step to improve stability
        // limit maximum step size relative to nVt_
        float maxStep = std::max(0.1f * nVt_, std::fabs(V) + 1.0f);
        if (dV > maxStep) dV = maxStep;
        if (dV < -maxStep) dV = -maxStep;

        V += dV;

        if (std::fabs(dV) < tol_)
            break;
    }

    // If Newton diverged to NaN or inf, fallback to a smooth clipper
    if (!std::isfinite(V))
    {
        // soft fallback: gentle tanh-style clipping
        float gain = 1.0f / (10.0f * nVt_);
        return std::tanh(Vin * gain) / gain;
    }

    return V;
}

