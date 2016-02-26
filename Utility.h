#pragma once

#include "stdafx.h"

const std::regex numberRegx = std::regex("[-+]?[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?"); //Fuck regular expression (implementerd in GCC 4.9 and i have 4.8.9 :'( )
bool isNumber(std::string s);
std::vector<std::string> split(std::string text, std::string delim);

void removeExtraSpace(std::string& s);
std::string replaceAll(std::string str, const std::string& from, const std::string& to, bool skipLiteral = false);

std::string loadFile(std::string path);

std::string toLower(std::string s);

double get_time();

template<class T> void remove(std::vector<T>& vector, const T& value)
{
    vector.erase(std::remove(vector.begin(), vector.end(), value), vector.end());
}

static std::default_random_engine utilityRandEngine;

template<class T> T rand(T min, T max)
{
    std::uniform_int_distribution<T> dist(min, max);
    return dist(utilityRandEngine);
}

template<class T> T rand(T max)
{
    std::uniform_int_distribution<T> dist(0, max);
    return dist(utilityRandEngine);
}

float rand(float min, float max);
float rand(float max);

bool randBool();

template<class T> void seed (T seed)
{
    utilityRandEngine.seed(seed);
}

//static string operator+(const string& s, const float& f)
//{
//    return string(s) + to_string(f);
//}
