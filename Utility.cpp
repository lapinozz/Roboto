#include "Utility.h"

bool isNumber(std::string s)
{
//    for(const char& c : s)
//    {
//        if(!std::isdigit(c, std::locale()) && c != '.' && c != '+' && c != '-') // not optimal buth good enough for now
//            return false;
//    }
//    return true;

    return std::regex_match(s, numberRegx);
}

std::vector<std::string> split(std::string text, std::string delim)
{
    int start = 0;
    int pos;
    std::vector<std::string> vec;
    while(text.length())
    {
        pos = text.find(delim);
        pos = pos == std::string::npos ? text.length() : pos;

        vec.push_back(text.substr(0, pos));
        text.erase(0, pos + 1);
        start = pos;
    }

    return vec;
}

void removeExtraSpace(std::string& s)
{
    int inText = 0;
    while(s.length() > 0 && s[0] == ' ')
        s.erase(0, 1);

    for(unsigned int x = 0; x < s.length(); x++)
    {
        size_t size = s.length();
        if(s[x] == '"')
        {
            size_t pos = s.find('"', x + 1);
            if(pos == std::string::npos)
                break;

            x = pos;
            continue;
        }

        if(s[x] == ' ')
        {
            while(x + 1 < s.length() && s[x + 1] == ' ')
                s.erase(x, 1);
        }
        else if(s[x] == '\n')
        {
            while(x + 1 < s.length() && (s[x + 1] == ' ' ||  s[x + 1] == '\n'))
                s.erase(x + 1, 1);

            if(x - 1 >= 0 && s[x - 1] == ' ')
                s.erase(--x, 1);
        }
    }

    if(s[s.length() - 1] == ' ')
        s.erase(s.length() - 1, 1);
}

std::string replaceAll(std::string str, const std::string& from, const std::string& to, bool skipLiteral)
{
    size_t pos = 0;
    size_t foundLiteralPos = 0;
    size_t literalPos;
    while((pos = str.find(from, pos)) != std::string::npos)
    {
        if(skipLiteral && ((literalPos = str.find('\"', foundLiteralPos)) < pos) )
        {
            pos = str.find('\"', literalPos + 1);
            foundLiteralPos = str.find('\"', pos + 1);

            if(pos++ == std::string::npos)
                break;

            continue;
        }

        str.replace(pos, from.length(), to);
        pos += to.length();
    }

    return str;
}

std::string loadFile(std::string path)
{
    std::ifstream is(path);
    std::string content;
    content.assign(std::istreambuf_iterator<char>(is), std::istreambuf_iterator<char>());
    return content;
}

std::string toLower(std::string s)
{
    for(char& c : s)
        c = std::tolower(c);

    return s;
}

float rand(float min, float max)
{
    std::uniform_real_distribution<float> dist(min, max);
    return dist(utilityRandEngine);
}

float rand(float max)
{
    std::uniform_real_distribution<float> dist(0, max);
    return dist(utilityRandEngine);
}

bool randBool()
{
    std::bernoulli_distribution dist;
    return dist(utilityRandEngine);
}

#ifdef __WINDOWS__

#include <windows.h>
double get_time()
{
    LARGE_INTEGER t, f;
    QueryPerformanceCounter(&t);
    QueryPerformanceFrequency(&f);
    return (double)t.QuadPart/(double)f.QuadPart;
}

#else


#endif
