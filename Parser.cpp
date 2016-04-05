#include "Parser.h"

#include "Utility.h"
#include "Assembler.h"

Parser::Parser()
{
//    std::cout << "1 : " << isNumber("1") << std::endl;
//    std::cout << "01234567890 : " << isNumber("01234567890") << std::endl;
//    std::cout << "+1-2+5 : " << isNumber("+1-2+5") << std::endl;
//    std::cout << "-7 : " << isNumber("-7") << std::endl;
//    std::cout << "+7 : " << isNumber("+7") << std::endl;
//    std::cout << "-99.78 : " << isNumber("-99.78") << std::endl;

//    loadFromFile("./code/code3-v2.SPML");
//    mMemory = Assembler::assemble(loadFile("./code/code3-v2.SPASM"));

//    parse();

    bindFunction("__getMemorySize", [&](){return getMemorySize();});
    bindFunction("__getCurrentAddr", [&](){return (uint32_t)this->mCurrentAddr;});

}

void Parser::parse()
{
    mAccumulator = 0;
    mCurrentAddr = 0;

//    for(size_t x = 0; x < mMemory.size(); x += 4)
//        std::cout << mMemory.at<uint32_t>(x) << std::endl;

    while(true)
    {
        uint32_t instruction = mMemory.at<uint32_t>(mCurrentAddr);

        const bool ptr  =                    instruction & PTR_MASK;//   >> PTR_SHIFT; since it get converted to bool it dont even need to be shifted
        Type type       = static_cast<Type>((instruction & TYPE_MASK)   >> TYPE_SHIFT);
        const OPP  opp  = static_cast<OPP> ((instruction & OPP_MASK )   >> OPP_SHIFT );
        int  operande   =                    instruction & OPERAND_MASK;

        if(ptr)
            operande = mMemory.at<int>(operande);

        bool isJmp = (opp == JMP) || (opp == JMPLESS) || (opp == JMPZERO);
        if(isJmp && type == JMP_RELATIVE_ADD)
            (operande = mCurrentAddr + operande), (type = INT);
        else if(isJmp && type == JMP_RELATIVE_SUB)
            (operande = mCurrentAddr - operande), (type = INT);

        if(type == INT && executeOPP<int>(opp, operande))
            break;
        else if(type == FLOAT && executeOPP<float>(opp, operande))
            break;
        else if(type == CHAR && executeOPP<char>(opp, operande))
            break;
        else if(type == UINT && executeOPP<uint32_t>(opp, operande))
            break;

        mCurrentAddr += 4;
    }
}

template<typename T>
bool Parser::executeOPP(const OPP& opp, const T& operande)
{
    if(opp == READ)
        std::cin >> mMemory.at<T>(operande);
    else if(opp == WRITE)
        std::cout << mMemory.at<T>(operande) << std::endl;
    else if(opp == LOAD)
        mAccumulator = mMemory.at<T>(operande);
    else if(opp == SAVE)
        mMemory.at<T>(operande) = mAccumulator;
    else if(opp == ADD)
        mAccumulator += mMemory.at<T>(operande);
    else if(opp == SUB)
        mAccumulator -= mMemory.at<T>(operande);
    else if(opp == DIV)
        mAccumulator /= mMemory.at<T>(operande);
    else if(opp == MUL)
        mAccumulator *= mMemory.at<T>(operande);
    else if(opp == JMP)
        mCurrentAddr = operande - 4; //mCurrentAddr is incremented at the end of the loop
    else if(opp == JMPLESS && mAccumulator < 0)
        mCurrentAddr = operande - 4;
    else if(opp == JMPZERO && mAccumulator == 0)
        mCurrentAddr = operande - 4;
    else if(opp == BOOL)
        mMemory.at<T>(operande) = (bool)mMemory.at<T>(operande);
    else if(opp == NOT)
        mMemory.at<T>(operande) = !(bool)mMemory.at<T>(operande);
    else if(opp == CALL_EXTERNAL)
    {
        uint32_t hash = mMemory.at<T>(operande);
        const BoxedFunction& func = mBindedFunctions[hash];
        std::vector<BoxedValue> args;

        loadFunctionArguments(func, args, mAccumulator);

        saveFunctionReturn(func(args));
    }
    else if(opp == LEA)
        mAccumulator = operande;
    else if(opp == END)
        return true;
    else
        throw std::runtime_error("Invalide opp");

    return false;
}

template<typename T>
static bool loadArgumentIfTypeMatch(const Type_Info& t, std::vector<BoxedValue>& args, Memory& memory, int& argAddr, size_t argSize = 4)
{
    if(t.bare_equal(user_type<T>()))
    {
        if(t.is_reference())
            args.push_back(BoxedValue(&memory.at<T>(argAddr)));
        else
            args.push_back(BoxedValue(memory.at<T>(argAddr)));

        argAddr += argSize;

        return true;
    }

    return false;
}

void Parser::loadFunctionArguments(const BoxedFunction& func, std::vector<BoxedValue>& args, int argAddr)
{
    for(Type_Info& t : func.getParameterType())
    {
        if(loadArgumentIfTypeMatch<int>(t, args, mMemory, argAddr));
        else if(loadArgumentIfTypeMatch<char>(t, args, mMemory, argAddr, 1));
        else if(loadArgumentIfTypeMatch<float>(t, args, mMemory, argAddr));
        else if(loadArgumentIfTypeMatch<unsigned int>(t, args, mMemory, argAddr));
    }
}

template<typename T>
static bool saveFunctionReturnIfMathType(const BoxedValue& returnValue, double& setTo)
{
    if(returnValue.match<T>())
    {
        setTo = returnValue.cast<T>();
        return true;
    }

    return false;
}

void Parser::saveFunctionReturn(const BoxedValue& returnValue)
{
    if(returnValue.match<void>())
        return;
    else if(saveFunctionReturnIfMathType<int32_t>(returnValue, mAccumulator));
    else if(saveFunctionReturnIfMathType<float>(returnValue, mAccumulator));
    else if(saveFunctionReturnIfMathType<char>(returnValue, mAccumulator));
    else if(saveFunctionReturnIfMathType<uint32_t>(returnValue, mAccumulator));
    else if(saveFunctionReturnIfMathType<int>(returnValue, mAccumulator));
    else if(saveFunctionReturnIfMathType<double>(returnValue, mAccumulator));
}

void Parser::bindFunction(std::string name, BoxedFunction func)
{
    mBindedFunctionHash[name] = murmurHash3_x86_32(name);
    mBindedFunctionNames[mBindedFunctionHash[name]] = name;
    bindFunction(mBindedFunctionHash[name], func);
}

void Parser::bindFunction(uint32_t id, BoxedFunction func)
{
    mBindedFunctions[id] = func;
}

void Parser::loadFromFile(std::string file)
{
    std::ifstream is(file);
    std::istream_iterator<int> start(is), end;
    std::vector<int> code(start, end);

    loadFromVector(code);
}

void Parser::loadFromMemory(const Memory& m)
{
    mMemory.set(m);
}

void Parser::loadFromVector(std::vector<std::string> vec)
{
    std::vector<int> code;
    for(const std::string& s : vec)
    {
        if(isNumber(s))
            code.push_back(std::atoi(s.c_str()));
        else
            throw("Parser: " + s + " is not a number");
    }

    loadFromVector(code);
}

void Parser::loadFromVector(std::vector<int> vec)
{
    mMemory.clear();
    mMemory.set(vec);
}

void Parser::loadFromString(std::string code, std::string separator)
{
    loadFromVector(split(code, separator));
}
