#include "Assembler.h"

#include "MathParser.h"

Assembler::Assembler()
{
}

Memory Assembler::assemble(const std::string& assembly)
{
    Memory memory;
    int currentMemory = 0;

    struct MiniSymbol
    {
        size_t flags;
        std::vector<std::string> toSolve;
        Parser::Type type;

        int constValue;
    };

//    struct ToSolve
//    {
//        size_t addr;
//    };

//    std::vector<std::pair<, std::vector<std::string>>> toSolve;
    std::vector<MiniSymbol> symbols;
    std::map<std::string, int> labels;

    std::vector<std::string> lines = split(assembly, "\n");
    memory.resize(lines.size() * 4 * 2);

    for(const std::string& line : lines)
    {
        std::vector<std::string> tokens = split(line, " ");
        for(std::string& token : tokens)
            token = toLower(token);

        uint32_t instruction = 0;
        bool data = false;

        Parser::Type type = Parser::UINT;

        if(tokens[0][tokens[0].size() - 1] == ':')
        {
            labels[tokens[0].substr(0, tokens[0].size() - 1)] = currentMemory;
            continue;
        }

        if(tokens[0] == "ptr")
        {
            instruction += Parser::PTR_MASK;
            tokens.erase(tokens.begin());
        }

        if(tokens[0] == "data")
            data = true;
        else if(tokens[0] == "read")
            instruction += Parser::READ << Parser::OPP_SHIFT;
        else if(tokens[0] == "write")
            instruction += Parser::WRITE << Parser::OPP_SHIFT;
        else if(tokens[0] == "load")
            instruction += Parser::LOAD << Parser::OPP_SHIFT;
        else if(tokens[0] == "save")
            instruction += Parser::SAVE << Parser::OPP_SHIFT;
        else if(tokens[0] == "add")
            instruction += Parser::ADD << Parser::OPP_SHIFT;
        else if(tokens[0] == "sub")
            instruction += Parser::SUB << Parser::OPP_SHIFT;
        else if(tokens[0] == "div")
            instruction += Parser::DIV << Parser::OPP_SHIFT;
        else if(tokens[0] == "mul")
            instruction += Parser::MUL << Parser::OPP_SHIFT;
        else if(tokens[0] == "jmp")
            instruction += Parser::JMP << Parser::OPP_SHIFT;
        else if(tokens[0] == "jlz")
            instruction += Parser::JMPLESS << Parser::OPP_SHIFT;
        else if(tokens[0] == "jmz")
            instruction += Parser::JMPZERO << Parser::OPP_SHIFT;
        else if(tokens[0] == "bool")
            instruction += Parser::BOOL << Parser::OPP_SHIFT;
        else if(tokens[0] == "not")
            instruction += Parser::NOT << Parser::OPP_SHIFT;
        else if(tokens[0] == "call")
            instruction += Parser::CALL_EXTERNAL << Parser::OPP_SHIFT;
        else if(tokens[0] == "end")
            instruction += Parser::END << Parser::OPP_SHIFT;
        else
            throw "error";

        tokens.erase(tokens.begin());

        if(tokens[0] == "data")
        {
            data = true;
            tokens.erase(tokens.begin());
        }

        if(tokens[0] == "int")
            type = Parser::INT, tokens.erase(tokens.begin());
        else if(tokens[0] == "float")
            type = Parser::FLOAT, tokens.erase(tokens.begin());
        else if(tokens[0] == "char")
            type = Parser::CHAR, tokens.erase(tokens.begin());
        else if(tokens[0] == "u_int" || tokens[0] == "uint")
            type = Parser::UINT, tokens.erase(tokens.begin());

//        if(data && !instruction)
//            symbols.push_back({currentMemory, tokens, type, false});
//        else
//        {
//            symbols.push_back({currentMemory, tokens, Parser::UINT, data});
//
//            memory.at<uint32_t>(currentMemory) = instruction + (type << Parser::TYPE_SHIFT);// + ((int)payload & Parser::OPERAND_MASK);
//            currentMemory += 4;
//        }

        if(instruction)
        {
            symbols.push_back({currentMemory, tokens, (data ? type : Parser::UINT), (data ? 1u : 0u)});
            memory.at<uint32_t>(currentMemory) = instruction + (type << Parser::TYPE_SHIFT);// + ((int)payload & Parser::OPERAND_MASK);
        }
        else
        {
            symbols.push_back({currentMemory, tokens, type, 2u});

            if(type == Parser::CHAR)
                currentMemory -= 3;
        }


        currentMemory += 4;
    }

    for(MiniSymbol& s : symbols)
    {
//        for(size_t flag : s.flags)

        std::string payloadString;
//        for(int x = 0; x < s.toSolve.size(); x++)
        for(auto& toSolve : s.toSolve)
        {
            if(labels[toSolve])
                toSolve = std::to_string(labels[toSolve]);

            payloadString += toSolve + " ";
        }

        double payload = MathParser::solve(MathParser::toPostfix(payloadString));

        if(s.constValue == 1)
        {
            memory.at<uint32_t>(s.flags) += currentMemory;

            if(s.type == Parser::INT)
                memory.at<int>(currentMemory) = payload;
            else if(s.type == Parser::FLOAT)
                memory.at<float>(currentMemory) = payload;
            else if(s.type == Parser::CHAR)
            {
                memory.at<char>(currentMemory) = payload;
                currentMemory -= 3; //we only want to increment of one and since it'll get incremented by four: 4-3=1. right?
            }
            else if(s.type == Parser::UINT)
                memory.at<unsigned int>(currentMemory) = payload;

            currentMemory += 4;
        }
        else if(s.constValue == 2)
        {
            if(s.type == Parser::INT)
                memory.at<int>(s.flags) = payload;
            else if(s.type == Parser::FLOAT)
                memory.at<float>(s.flags) = payload;
            else if(s.type == Parser::CHAR)
                memory.at<char>(s.flags) = payload;
            else if(s.type == Parser::UINT)
                memory.at<uint32_t>(s.flags) = payload;
        }
        else if(s.constValue == 0)
            memory.at<uint32_t>(s.flags) += payload;
    }

//    for(std::pair<size_t, std::vector<std::string>>& pair : toSolve)
//    {
//        std::string payloadString;
//        for(int x = 0; x < pair.second.size(); x++)
//        {
//            if(labels[pair.second[x]])
//                pair.second[x] = std::to_string(labels[pair.second[x]]);
//
//            payloadString += pair.second[x] + " ";
//        }
//
//        memory.at<uint32_t>(pair.first) =
//    }

    memory.resize(currentMemory);

    return memory;
}

std::string Assembler::disassemble(Memory& code)
{

}
