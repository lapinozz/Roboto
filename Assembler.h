#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include "Parser.h"

class Assembler
{
public:
    static Memory assemble(const std::string& assembly);
    static std::string disassemble(Memory& code);

private:
    Assembler();
};

#endif // ASSEMBLER_H
