#ifndef _AUDIO_ENGINE_H_
#define _AUDIO_ENGINE_H_

#include "fmod.hpp"
#include "fmod_errors.h"
#include "vendors/json.hpp"

#include <fstream>
#include <iostream>
#include <map>
#include <math.h>
#include <string>
#include <vector>


using json = nlohmann::json;

struct data
{
    std::vector<float> embedding;
    std::string path;
    int chunk;
};

#endif