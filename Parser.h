#ifndef PARSER_H
#define PARSER_H

#include "stdafx.h"

#include "Memory.h"
#include "Hash.hpp"

#include "BoxedFunction.hpp"

class Compiler;
class Assembler;
class Linker;

class Parser
{
    friend Compiler;
    friend Assembler;
    friend Linker;

    static const int TO_CODE              = 1000;
    static const int OPERATION_CODE_SHIFT = 24;
    static const int PTR_MASK             = 0x80000000, PTR_SHIFT  = 31;
    static const int TYPE_MASK            = 0x60000000, TYPE_SHIFT = 29;
    static const int OPP_MASK             = 0x1F000000, OPP_SHIFT  = 24;
    static const int OPERAND_MASK         = 0x00FFFFFF;

    enum OPP
    {
        READ           = 0, //For now it use command line I/O but latter it will ue wire to connect to other commponent
        WRITE          = 1, //    ^Same^

        LOAD           = 4,
        SAVE           = 5,

        ADD            = 6,
        SUB            = 7,
        DIV            = 8,
        MUL            = 9,

        JMP            = 10,
        JMPLESS        = 11, // JMP if accumulator is less then zero
        JMPZERO        = 12, // JMP if accumulator is zero

        BOOL           = 13,
        NOT            = 14, //BOOL OPP not binary

        //experimental
        PTR_SET_OFFSET = 15,


        //operand shall(I like that word) be and address (or pointer to address) to function ID
        //Accumulator shall contain address  of argument
        CALL_EXTERNAL  = 16,

        END            = 17
    };

    enum Type {INT = 0, FLOAT = 1, CHAR = 2, UINT = 3,
               JMP_RELATIVE_ADD = FLOAT,
               JMP_RELATIVE_SUB = CHAR
              };

public:
    Parser();

    void parse();

    uint32_t getMemorySize(){return mMemory.size();};
    void setMemorySize(uint32_t size){mMemory.resize(size);};

    template<typename T>
    bool executeOPP(const OPP& opp, const T& operande);

    void loadFunctionArguments(const BoxedFunction& func, std::vector<BoxedValue>& args, int argAddr);
    void saveFunctionReturn(const BoxedValue& returnValue);

    void loadFromFile(std::string file);
    void loadFromMemory(const Memory& m);
    void loadFromVector(std::vector<std::string> vec);
    void loadFromVector(std::vector<int> vec);
    void loadFromString(std::string code, std::string separator = "\n");

    void bindFunction(std::string name, BoxedFunction func);
    void bindFunction(uint32_t id, BoxedFunction func);

    template<typename T, typename... Params>
    void bindFunction(std::string name, T t, Params... params)
    {
        bindFunction(name, BoxedFunction(std::move(t), std::move(params)...));
    }

    template<typename T, typename... Params>
    void bindFunction(uint32_t id, T t, Params... params)
    {
        bindFunction(id, BoxedFunction(std::move(t), std::move(params)...));
    }

protected:
private:
    double mAccumulator;
    int mCurrentAddr;

    std::map<uint32_t, BoxedFunction> mBindedFunctions;
    std::map<std::string, uint32_t> mBindedFunctionHash;
    std::map<uint32_t, std::string> mBindedFunctionNames;

    Memory mMemory;
};

#endif // PARSER_H
