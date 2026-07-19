#ifndef PRESETS_H
#define PRESETS_H

#include <string>

struct Config;

void applyPreset(const std::string &preset, Config &conf);

#endif // PRESETS_H
