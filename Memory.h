#ifndef MEMORY_H
#define MEMORY_H

#include "Utility.h"

class Memory
{
public:
    Memory();
    ~Memory();

    void resize(size_t newSize);
    void clear();
    size_t size();

    void set(const Memory& m, size_t position = 0);

    template<typename T,
             typename = std::enable_if_t<std::is_integral<T>::value || std::is_same<float, typename std::remove_cv<T>::type>::value>>
    T& at(size_t addresse)
    {
        return *(T*)(mBuffer.data() + addresse);
    }

    template<typename T,
             typename = std::enable_if_t<std::is_integral<T>::value || std::is_same<float, typename std::remove_cv<T>::type>::value>>
    void operator=(const std::vector<T>& vec)
    {
        resize(vec.size() * sizeof(T));
        for(size_t x = 0; x < vec.size(); x++)
            at<T>(x * sizeof(T)) = vec[x];
    }

    template<typename T,
             typename = std::enable_if_t<std::is_integral<T>::value || std::is_same<float, typename std::remove_cv<T>::type>::value>>
    void set(const std::vector<T>& vec, size_t position = 0)
    {
//        resize(vec.size() * sizeof(T));
//        for(size_t x = 0; x < vec.size(); x++)
//            at<T>(x * sizeof(T)) = vec[x];

        if(size() < vec.size()*sizeof(T))
            resize(position + vec.size()*sizeof(T));

        for(size_t x = 0; x < vec.size(); x++)
            at<char>(position + x*sizeof(T)) = vec[x];
    }

private:
    std::vector<char> mBuffer;
};

#endif // MEMORY_H
