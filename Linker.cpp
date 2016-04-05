#include "Linker.h"

Linker::Linker()
{
    //ctor
}

Linker::~Linker()
{
    //dtor
}

Memory Linker::link(std::vector<Compiler::CompiledData>& compiledDatas)
{
    Memory memory;
    std::map<std::string, Compiler::Symbol> symbols;

    memory.resize(4*4);
    memory.at<uint32_t>(0*4) = Parser::LOAD << Parser::OPP_SHIFT;
    memory.at<uint32_t>(1*4) = Parser::SAVE << Parser::OPP_SHIFT;
    memory.at<uint32_t>(2*4) = Parser::JMP  << Parser::OPP_SHIFT;
    memory.at<uint32_t>(3*4) = Parser::END  << Parser::OPP_SHIFT;

    symbols["12"] = Compiler::Symbol(Compiler::Symbol::CONST, "12");
    symbols["FUNCTION_main_RETURN"] = Compiler::Symbol(Compiler::Symbol::VAR, "FUNCTION_main_RETURN");
    symbols["FUNCTION_main"] = Compiler::Symbol(Compiler::Symbol::VAR, "FUNCTION_main");

    symbols["12"].flagLines.push_back(0*4);
    symbols["FUNCTION_main_RETURN"].flagLines.push_back(1*4);
    symbols["FUNCTION_main"].flagLines.push_back(2*4);

//    symbols["VARIABLE_programSize"] = Compiler::Symbol(Compiler::Symbol::VAR, "VARIABLE_programSize");

    int currentAddr = memory.size();

    for(Compiler::CompiledData& data : compiledDatas)
    {
        size_t oldSize = currentAddr;
        memory.resize(oldSize + data.memory.size());
        for(; currentAddr < memory.size(); currentAddr++)
            memory.at<char>(currentAddr) = data.memory.at<char>(currentAddr - oldSize);

        for(auto& s : data.symbols)
        {
            s.second += oldSize;
            s.second.eval(symbols);

            if(!!s.second) //lazy me
                symbols[s.first] += s.second;
        }
    }

    memory.resize(memory.size() + symbols.size()*4);

    for(auto& pair : symbols)
    {
        Compiler::Symbol& s(pair.second);

        if(s.flagLines.empty())
            continue;

        if(s.addr == -1)
        {
            s.addr = currentAddr;
            memory.at<uint32_t>(currentAddr) = (s.type == Compiler::Symbol::CONST ? std::atoll(s.name.c_str()) : 0);
            currentAddr += 4;
        }

        for(int addr : s.flagLines)
            memory.at<uint32_t>(addr) += s.addr;

        std::cout << "Symbol: " << pair.first << " addr " << s.addr << std::endl;
    }

    memory.resize(currentAddr);

    if(!!symbols["__programSize"]) //lazy me
        memory.at<uint32_t>(symbols["__programSize"].addr) = currentAddr;

//    for(unsigned int x = 0; x < memory.size(); x += 4)
//        std::cout << x << ": " << memory.at<int>(x) << std::endl;

    return memory;
}

