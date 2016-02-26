#ifndef LINKER_H
#define LINKER_H

#include "Compiler.h"

class Linker
{
    public:
        static Memory link(std::vector<Compiler::CompiledData>& compiledDatas);
    protected:
    private:
        Linker();
        virtual ~Linker();
};

#endif // LINKER_H
