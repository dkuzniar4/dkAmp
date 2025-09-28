/*
  ==============================================================================

    ShapeProcessor.h
    Created: 28 Sep 2025 4:55:00pm
    Author:  dkuzn

  ==============================================================================
*/

#pragma once
#include <cstdint>
#include <cmath>
#include <algorithm>
#include <vector>

class ShapeProcessor
{
public:
    ShapeProcessor() = default;

    // Load LUT
    void load(float* tab);

    // Init processor
    void init(uint32_t ampSteps);

    // Process data
    float process(float input);

private:

    float* tab = nullptr;
    uint32_t ampSteps = 0;

};
