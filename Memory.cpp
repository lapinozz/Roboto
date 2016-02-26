#include "Memory.h"

Memory::Memory()
{
}

Memory::~Memory()
{
}

void Memory::clear()
{
    resize(0);
}

void Memory::resize(size_t newSize)
{
    mBuffer.resize(newSize);
}

size_t Memory::size()
{
    return mBuffer.size();
}

void Memory::set(const Memory& m, size_t position)
{
//    if(size() < m.size())
//        m.resize(position + m.size())
//
//        for(size_t x = 0; x < m.size(); x++)
//            at<char>(position + x) = m.at<char>(x);

    set(m.mBuffer, position);
}

