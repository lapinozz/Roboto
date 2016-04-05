#include "Compiler.h"

#include "Utility.h"

#include "Parser.h"
#include "MathParser.h"

Compiler::Compiler()
{
//    loadFromFile("./code3.roboto");
//    for(std::string line : mProgram)
//        std::cout << line << std::endl;

//    compile();
//    mCurrentAddr = 0;
//    addInstruction(Parser::WRITE, solve(MathParser::toPostfix("(1 || 0) && 7 < 8 && 41--2 == 43 && (7-3)*2 == 8 && 0 == !1 && 1 == !0 && (!(!1&&0) && (!1 || 1))")));
//    addInstruction(Parser::END);
//    Parser p;
//
//    for(auto& pair : mSymbols)
//    {
//        Symbol& s(pair.second);
//
//        if(s.addr == -1)
//            s.addr = mCurrentAddr++;
//
//        mMemory.push_back(s.type == Symbol::CONST ? std::atoi(s.name.c_str()) : 0);
//        for(int addr : s.flagLines)
//        {
//            mMemory[addr] += s.addr;
//        }
//    }
//
//    p.loadFromVector(mMemory);
//
//    for(int i : mMemory)
//        std::cout << i << std::endl;
//
//    std::cout << mMemory.size() << std::endl;
//
//    p.parse();
}

void Compiler::preProcessor()
{

}

void Compiler::preParse()
{
    for(size_t x = 0u; x < mProgram.length() && mProgram.length() > 0; x++)
    {
        if(mProgram[x] == '\"')
        {
            auto pos = mProgram.find('\"', x + 1);
            mStringConst[mCurrentStringConst] = mProgram.substr(x + 1, pos - x - 1);
            mProgram.replace(x, (pos - x) + 1, "STRING_CONST_" + std::to_string(mCurrentStringConst));
            mCurrentStringConst++;
        }

        MathParser::OPP opp = MathParser::toOPP(mProgram.substr(x, 2));

        if(opp == MathParser::COMMENT)
        {
            mProgram.erase(x, mProgram.find('\n', x) - x);
            continue;
        }
    }


    mProgram = replaceAll(mProgram, "\n", "", true);

    for(size_t x = 0u; x < mProgram.length() && mProgram.length() > 0; x++)
    {
        if(mProgram[x] == '\"')
            x = mProgram.find('\"', x + 1);

        MathParser::Operator opp;
        for(size_t size = MathParser::MAX_OPP_SIZE; size > 0; size--)
        {
            opp = MathParser::toOPP(mProgram.substr(x, size));
            if(opp != MathParser::UNDEFINED)
                break;
        }

        if(opp != MathParser::UNDEFINED)
        {
            mProgram.insert(x + opp.str.size(), " ");
            mProgram.insert(x, " ");
            x += opp.str.size() + 1;
        }
    }

    removeExtraSpace(mProgram);

    std::vector<std::string> tokens(split(mProgram, " "));
    mProgram.clear();
    for(unsigned int x = 0; x < tokens.size(); x++)
    {
        if(tokens[x] ==  ";")
            tokens[x] = "\n";
        else if(tokens[x] == "{")
            tokens.insert(tokens.begin() + x + 1, "\n");
        else if(tokens[x] == "}")
        {
            if(tokens.size() > x + 1 && tokens[x + 1] == "else")
            {
                tokens.erase(tokens.begin() + x--);
                continue;
            }
            else if(tokens.size() > x + 1)
                tokens.insert(tokens.begin() + x + 1, "\n");
        }
        else if(isScopedKeyword(toKeyword(tokens[x])))
        {
            Keyword keyword = toKeyword(tokens[x]);

            bool started = false;
            int deep = 0;

            if(tokens[x + 1] == "(" || tokens[x + 2] == "(")
            {
                for(; x < tokens.size() && (!started || deep); x++)
                {
                    if(tokens[x] == "(")
                        deep++, started = true;
                    else if(tokens[x] == ")")
                        deep--;
                }
            }
            else
                x++;

            if(keyword == FUNCTION)
            {
                if(tokens[x] == "{")
                    x++;
                else if(toKeyword(tokens[x]) == EXTERN)
                    x += 2;
            }
            else if(tokens[x] != "{")
            {
                tokens.insert(tokens.begin() + x, "{");
                tokens.insert(std::find(tokens.begin() + x, tokens.end(), ";") + 1, "}");
            }

            tokens.insert(tokens.begin() + x, "\n");
        }
    }

    for(std::string& s : tokens)
        mProgram += s + " ";

    removeExtraSpace(mProgram);
}

Compiler::CompiledData Compiler::compile()
{
    mCurrentStringConst = 0;
    mStringConst.clear();

    mCurrentAddr = 0;
    mCurrentContext = 0;
    while(!mContext.empty())
        mContext.pop();

    mDatas.memory.clear();
    mDatas.symbols.clear();

//    addInstruction(Parser::LOAD, "12");
//    addInstruction(Parser::SAVE, "FUNCTION_main_RETURN");
//    addInstruction(Parser::JMP, "FUNCTION_main");
//    addInstruction(Parser::END, 0);

    preProcessor();
    preParse();

    std::cout << mProgram << std::endl;

    for(std::string line : split(mProgram, "\n"))
    {
        removeExtraSpace(line);

        std::vector<std::string> tokens(split(line, " "));
//        for(std::string token : tokens)
//            std::cout << token << " ";
//        std::cout << std::endl;

        if(tokens.size() == 0) continue;

        if(tokens[0] == "input")
            addInstruction(Parser::READ, tokens[1]);
        if(tokens[0] == "inputc")
            addInstruction(Parser::READ, tokens[1], Parser::CHAR);
        else if(tokens[0] == "print")
            addInstruction(Parser::WRITE, solve(MathParser::toPostfix(line.substr(5))));
        else if(tokens[0] == "printc")
            addInstruction(Parser::WRITE, tokens[1], Parser::CHAR);
        else if(tokens[0] == "else" || tokens[0] == "if" || tokens[0] == "while" || tokens[0] == "function")
        {
            if(tokens[0] == "if")
                pushContext(Context::CONDITION);
            else if(tokens[0] == "while")
                pushContext(Context::WHILE);
            else if(tokens[0] == "function")
            {
                bool implementation = (tokens.back() == "{");

                if(implementation)
                    tokens.erase(tokens.end());

                Function::FunctionSignature signature;

                std::string functionName = tokens[1];

                tokens.erase(tokens.begin(), tokens.begin() + 2);

                if(tokens[0] != "(")
                    throw std::runtime_error("expected \"(\" not \"" + tokens[0] + "\"");

                tokens.erase(tokens.begin());

                while(true)
                {
                    if(tokens[0] == ")")
                    {
                        tokens.erase(tokens.begin());
                        break;
                    }

                    if(!isTypeKeyword(tokens[0]) && tokens[0] != ")")
                        throw std::runtime_error("Exptected type or \")\" not \"" + tokens[0] + "\"");

                    signature.params.push_back({tokens[1], toType(tokens[0])});

                    if(!findVariable(tokens[1]))
                        addVariable(tokens[1], toType(tokens[0]));

                    if(tokens[2] != "," && tokens[2] != ")")
                        throw std::runtime_error("Expected \",\" or \")\" not \"" + tokens[2] + "\"");

                    tokens.erase(tokens.begin(), tokens.begin() + 3 - (tokens[2] == ")"));

                }

                if(!findFunction(functionName, signature))
                    addFunction(functionName, signature);

                Function& f = findFunction(functionName, signature);

                if(tokens.size() > 0)
                {
                    if(tokens[0] == "extern")
                    {
                        if(f.isExtern)
                            throw std::runtime_error("Function \"" + functionName + "\" already declared as extern");

                        if(tokens[1].substr(0, 13) == "STRING_CONST_")
                            f.externName = mStringConst[std::atoi(tokens[1].substr(13).c_str())];
                        else
                            throw std::runtime_error("Expected string constant not  \"" + tokens[1] + "\"");

                        f.externHash = murmurHash3_x86_32(f.externName);

                        f.isExtern = true;
                    }
                    else
                        throw std::runtime_error("Exptected nothing or \"extern\" not \"" + tokens[0] + "\"");
                }

                if(implementation)
                {
                    if(f.implemented)
                        throw std::runtime_error("Function \"" + f.name + "\" already implemented");

                    if(f.isExtern)
                        throw std::runtime_error("Function \"" + f.name + "\" is extern, cannot implement it");

                    f.implemented = true;

                    pushContext(Context::FUNCTION);
                    mContext.top().functionName = functionName;
                    addSymbole(f.symbolName, mCurrentAddr, Symbol::LABEL);
                    mCurrentFunction = f;

                    for(unsigned int x = 0; x < f.signature.params.size(); x++)
                    {
                        addInstruction(Parser::LOAD, f.symbolName + "_PARAM_" + std::to_string(x));
                        addInstruction(Parser::SAVE, f.signature.params[x].name);
                    }
                }
                else
                    continue;

            }
            else if(tokens[0] == "else")
            {
                addInstruction(Parser::JMP, getContextEnd());
                tokens.erase(tokens.begin());
            }

            addContextPart(mCurrentAddr);

            if(tokens[0] == "if" || tokens[0] == "while")
            {
                int start = -1;
                for(unsigned int x = 1; x < line.length() && start == -1; x++)
                    start = line[x] == '(' ? x + 1: -1;
                int end = -1;
                for(int x = line.length(); x > 0 && end == -1; x--)
                    end = line[x] == ')' ? x - 1: -1;

                if(start == -1 || end == -1)
                    throw "let's crash the world";

                std::string tmp = solve(MathParser::toPostfix(line.substr(start, end - start)));
                addInstruction(Parser::LOAD, tmp);
                addInstruction(Parser::JMPZERO, getContextPart(mContext.top().nextPart));
            }
        }
        else if(tokens[0] == "{");
//        else if(tokens[0] == "end")
        else if(tokens[0] == "}")
        {
            if(mContext.top().type == Context::WHILE)
                addInstruction(Parser::JMP, getContextPart(0));
            else if(mContext.top().type == Context::FUNCTION)
            {
                addContextPart(mCurrentAddr);
                addInstruction(Parser::LOAD, std::to_string(Parser::JMP << Parser::OPP_SHIFT));
                addInstruction(Parser::ADD, mCurrentFunction.symbolName + "_RETURN");
//                addInstruction(Parser::SAVE, mCurrentAddr + 1*4);
                addFlag("CURRENT_ADDR", mCurrentAddr);
                addInstruction(Parser::SAVE, 1*4);
                addInstruction(Parser::JMP, 00000);
            }

            if(mContext.top().type != Context::FUNCTION)
                addContextPart(mCurrentAddr);

            popContext();
        }
        else if(isTypeKeyword(tokens[0])) //variable definition
        {
            addVariable(tokens[1], toType(tokens[0]));
            solve(MathParser::toPostfix(line.substr(tokens[0].size())));
        }
        else if(tokens[0] == "goto")
        {
            addFlag(tokens[1], mCurrentAddr);
            addInstruction(Parser::JMP, 0);
        }
        else if(tokens[0] == "exit")
        {
            addInstruction(Parser::END, 0);
        }
        else if(tokens[0] == "return")
        {
            if(tokens.size() > 1)
            {
                addInstruction(Parser::LOAD, solve(MathParser::toPostfix(line.substr(std::string("return").length() + 1))));
                addInstruction(Parser::SAVE, getFunctionReturn());
            }
            addInstruction(Parser::JMP, getFunctionEnd());
        }
        else if(tokens[0].substr(0, 1) == ":")//label
        {
            tokens[0].erase(0, 1);
            addSymbole(tokens[0], mCurrentAddr, Symbol::CONST);

        }
//        else if(tokens.size() >= 2 && tokens[1] == "=")
//        {
//            solve(MathParser::toPostfix(line));
////            addInstruction(Parser::LOAD, solve(MathParser::toPostfix(line.substr(tokens[0].length() + 3))));
////            addInstruction(Parser::SAVE, tokens[0]);
//        }
//        else if(tokens.size() >= 2 && tokens[1] == "(")//function call
//        {
//            solve(MathParser::toPostfix(line));
//        }
        else
        {
            solve(MathParser::toPostfix(line));
        }
    }

    return mDatas;
}

void Compiler::pushContext(Context::Type type)
{
    mContext.push(Context(type, mCurrentContext++));
}

void Compiler::popContext()
{
    mContext.pop();
}

std::string Compiler::getContext(int id)
{
    if(id == -1)
        id = mContext.top().contextID;

    return mCurrentFunction.symbolName + "_CONTEXT_" + std::to_string(id);
}

std::string Compiler::getContextPart(int part)
{
    return getContext() + "_PART_" + std::to_string(part);
}

std::string Compiler::getContextEnd()
{
    return getContext() + "_END";
}

std::string Compiler::addContextPart(int addr)
{
    addSymbole(getContextEnd(), addr, Symbol::LABEL);
    addSymbole(getContextPart(mContext.top().nextPart++), addr, Symbol::LABEL);
    return getContextPart(mContext.top().nextPart-1);
}

std::string Compiler::getFunctionReturn()
{
//    std::string returnSymbol;

//    std::stack<Context> s;
//    while(!mContext.empty() && mContext.top().type != Context::FUNCTION)
//    {
//        s.push(mContext.top());
//        mContext.pop();
//    }
//
//    if(mContext.top().type == Context::FUNCTION)
//        returnSymbol = "FUNCTION_" + mContext.top().functionName + "_VALUE";
//
//    while(!s.empty())
//    {
//        mContext.push(s.top());
//        s.pop();
//    }

//    return returnSymbol;

    return mCurrentFunction.symbolName + "_VALUE";
}

std::string Compiler::getFunctionEnd()
{
    std::string functionEnd;

    std::stack<Context> s;
    while(!mContext.empty() && mContext.top().type != Context::FUNCTION)
    {
        s.push(mContext.top());
        mContext.pop();
    }

    if(mContext.top().type == Context::FUNCTION)
        functionEnd =  getContextEnd();

    while(!s.empty())
    {
        mContext.push(s.top());
        s.pop();
    }

    return functionEnd;
}

std::string Compiler::solve(std::string postfix)
{
    mCurrentSolverTemp = 0;

    removeExtraSpace(postfix);
    std::vector<std::string> tokens(split(postfix, " "));

    std::cout << "solve : " << postfix << std::endl;

    std::stack<std::string> s;
    for(std::string& token : tokens)
    {
        auto opp = MathParser::toOPP(token);
        if(opp != MathParser::UNDEFINED)
        {
            std::string x = s.top();
            s.pop();

            std::string y = "";
            if(opp.unary == MathParser::BINARY)
            {
                y = s.top();
                s.pop();
            }

            s.push(solveOPP(opp, y, x));
//            std::cout << y << " " << opp << " " << x << " = " << s.top() << std::endl;
        }
        else
        {
            if(isNumber(token))
//                addSymbole(token, Symbol::CONST);
                addConst(token);
            else
                findSymbol(token);
//            else if(!mDatas.symbols[token])
//                throw std::runtime_error("Symbol \"" + token + "\" not defined");

            s.push(token);
        }
    }

    while(!mSolverFunctions.empty())
        mSolverFunctions.pop();

    if(s.top()[0] == '*')
    {
        std::string tmp = "SOLVER_TMP_" + std::to_string(mCurrentSolverTemp++);
        addInstruction(Parser::LOAD, s.top().substr(1), Parser::INT, true);
        addInstruction(Parser::SAVE, tmp);
        s.top() = tmp;
    }

    return s.top();
}

std::string Compiler::solveOPP(MathParser::Operator opp, std::string val1, std::string val2)
{
    std::string tmp = "SOLVER_TMP_" + std::to_string(mCurrentSolverTemp++);

    bool ptr1 = val1[0] == '*';
    bool ptr2 = val2[0] == '*';

    if(ptr1)
        val1 = val1.substr(1);
    if(ptr2)
        val2 = val2.substr(1);

    if(opp == MathParser::ADD)
    {
        addInstruction(Parser::LOAD, val1, Parser::INT, ptr1);
        addInstruction(Parser::ADD, val2, Parser::INT, ptr2);
        addInstruction(Parser::SAVE, tmp);
    }
    else if(opp == MathParser::SUB)
    {
        addInstruction(Parser::LOAD, val1, Parser::INT, ptr1);
        addInstruction(Parser::SUB, val2, Parser::INT, ptr2);
        addInstruction(Parser::SAVE, tmp);
    }
    else if(opp == MathParser::MUL)
    {
        addInstruction(Parser::LOAD, val1, Parser::INT, ptr1);
        addInstruction(Parser::MUL, val2, Parser::INT, ptr2);
        addInstruction(Parser::SAVE, tmp);
    }
    else if(opp == MathParser::DIV)
    {
        addInstruction(Parser::LOAD, val1, Parser::INT, ptr1);
        addInstruction(Parser::DIV, val2, Parser::INT, ptr2);
        addInstruction(Parser::SAVE, tmp);
    }
//    else if(opp == EXP) // need to add SML OPP
//        return pow(val1, val2);
//    else if(opp == MOD) // need to add SML OPP
//        return (int)val1 % (int)val2;
    else if(opp == MathParser::OR)
    {
        addInstruction(Parser::LOAD, val1, Parser::INT, ptr1);
        addInstruction(Parser::JMPZERO, 2*4, Parser::JMP_RELATIVE_ADD);
        addInstruction(Parser::JMP, 2*4, Parser::JMP_RELATIVE_ADD);
        addInstruction(Parser::LOAD, val2);
        addInstruction(Parser::SAVE, tmp);
    }
    else if(opp == MathParser::AND)
    {
        addInstruction(Parser::LOAD, val1, Parser::INT, ptr1);
        addInstruction(Parser::JMPZERO, 3*4, Parser::JMP_RELATIVE_ADD);
        addInstruction(Parser::LOAD, val2, Parser::INT, ptr2);
        addInstruction(Parser::SAVE, tmp);
    }
    else if(opp == MathParser::COMPARE_EQUALITY)
    {
        addInstruction(Parser::LOAD, val1, Parser::INT, ptr1);
        addInstruction(Parser::SUB, val2, Parser::INT, ptr2);
        addInstruction(Parser::SAVE, tmp);
        addInstruction(Parser::NOT, tmp);
    }
    else if(opp == MathParser::COMPARE_NOT_EQUALE)
    {
        addInstruction(Parser::LOAD, val1, Parser::INT, ptr1);
        addInstruction(Parser::SUB, val2, Parser::INT, ptr2);
        addInstruction(Parser::SAVE, tmp);
        addInstruction(Parser::BOOL, tmp);
    }
    else if(opp == MathParser::COMPARE_LESS)
    {
        addInstruction(Parser::LOAD, val1, Parser::INT, ptr1);
        addInstruction(Parser::SUB, val2, Parser::INT, ptr2);
        addInstruction(Parser::ADD, "1");
        addInstruction(Parser::SAVE, tmp);
        addInstruction(Parser::JMPLESS, 2*4, Parser::JMP_RELATIVE_ADD);
        addInstruction(Parser::NOT, tmp);
    }
    else if(opp == MathParser::COMPARE_GREATER)
    {
        addInstruction(Parser::LOAD, val2), Parser::INT, ptr2;
        addInstruction(Parser::SUB, val1, Parser::INT, ptr1);
        addInstruction(Parser::ADD, "1");
        addInstruction(Parser::SAVE, tmp);
        addInstruction(Parser::JMPLESS, 2*4, Parser::JMP_RELATIVE_ADD);
        addInstruction(Parser::NOT, tmp);
    }
    else if(opp == MathParser::COMPARE_LESS_OR_EQUALE)
    {
        addInstruction(Parser::LOAD, val1, Parser::INT, ptr1);
        addInstruction(Parser::SUB, val2, Parser::INT, ptr2);
        addInstruction(Parser::SAVE, tmp);
        addInstruction(Parser::JMPLESS, 2*4, Parser::JMP_RELATIVE_ADD);
        addInstruction(Parser::NOT, tmp);
    }
    else if(opp == MathParser::COMPARE_GREATER_OR_EQUALE)
    {
        addInstruction(Parser::LOAD, val2, Parser::INT, ptr2);
        addInstruction(Parser::SUB, val1, Parser::INT, ptr1);
        addInstruction(Parser::SAVE, tmp);
//        addInstruction(Parser::JMPLESS, mCurrentAddr + 2);
        addInstruction(Parser::JMPLESS, 2*4, Parser::JMP_RELATIVE_ADD);
        addInstruction(Parser::NOT, tmp);
    }
    else if(opp == MathParser::FUNCTION_PARAM)
    {
        if(mSolverFunctions.empty() || mSolverFunctions.top().name != val1)
            mSolverFunctions.push(Function(val1, {}));

        mSolverFunctions.top().signature.params.push_back({val2, findVariable(val2).variableType});
//        mCurrentSolverTemp--;
        tmp = val1;
    }
    else if(opp == MathParser::FUNCTION_CALL)
    {
        for(int x = 0; x < mCurrentSolverTemp; x++)
        {
            addInstruction(Parser::LOAD, "SOLVER_TMP_" + std::to_string(x));
            addInstruction(Parser::SAVE, getContext() + "_SOLVER_TMP_" + std::to_string(x));
        }

        Function f;

        if(!mSolverFunctions.empty() && mSolverFunctions.top().name == val1)
            f = findFunction(val1, mSolverFunctions.top().signature);
        else
            f = findFunction(val1, {});

        if(!f)
            throw std::runtime_error("Function \"" + val1 + "\" not declared");

        if(f.isExtern)
        {
            addInstruction(Parser::CALL_EXTERNAL, std::to_string(f.externHash), Parser::UINT);
            addInstruction(Parser::SAVE, tmp);
        }
        else
        {
            for(unsigned int x = 0; (!mSolverFunctions.empty() && mSolverFunctions.top().name == val1) && x < mSolverFunctions.top().signature.params.size(); x++)
            {
                addInstruction(Parser::LOAD,  mSolverFunctions.top().signature.params[x].name);
                addInstruction(Parser::SAVE, f.symbolName + "_PARAM_" + std::to_string(x));
            }

//        addInstruction(Parser::LOAD,  std::to_string(mCurrentAddr + 3*4 + 4*4)); //problem is you dont know the offset (here's 4*4) at this compile stage
            addInstruction(Parser::LOAD, "$CURRENT_ADDR + 3*4"); ///TODO haha this is the only use case, replace it with a smarter idea, should probably add vm instruction to get current address
            addInstruction(Parser::SAVE, f.symbolName + "_RETURN");
            addInstruction(Parser::JMP, f.symbolName);

            for(int x = 0; x < mCurrentSolverTemp; x++)
            {
                addInstruction(Parser::LOAD, getContext() + "_SOLVER_TMP_" + std::to_string(x));
                addInstruction(Parser::SAVE, "SOLVER_TMP_" + std::to_string(x));
            }

            tmp = f.symbolName + "_VALUE";
        }

        if(!mSolverFunctions.empty() && mSolverFunctions.top().name == val1)
            mSolverFunctions.pop();
    }
    else if(opp == MathParser::ASSIGNMENT)
    {
//        addInstruction(Parser::LOAD, val1);
        addInstruction(Parser::LOAD, val2, Parser::INT, ptr2);
//        addInstruction(Parser::SAVE, val2);
        addInstruction(Parser::SAVE, val1, Parser::INT, ptr1);
        tmp = val1;
    }
    ///TODO: for unary operator the value to use is val2, not so good, might make a second function to be sure to have no confusion

    else if(opp == MathParser::UNARY_MINUS)
    {
        addInstruction(Parser::LOAD, "0");
        addInstruction(Parser::SUB, val2, Parser::INT, ptr2);
        addInstruction(Parser::SAVE, tmp);
    }
    else if(opp == MathParser::UNARY_PLUS)
        throw std::runtime_error("Implement unary plus, you fool");
    else if(opp == MathParser::NOT)
    {
        addInstruction(Parser::LOAD, val2, Parser::INT, ptr2);
        addInstruction(Parser::SAVE, tmp);
        addInstruction(Parser::NOT, tmp);
    }
    else if(opp == MathParser::REFERENCE)
    {
        addInstruction(Parser::LEA, val2, Parser::INT, ptr2);
        addInstruction(Parser::SAVE, tmp);
//        addInstruction(Parser::LOAD, "$CURRENT_ADDR - 4");
//        addInstruction(Parser::SUB , std::to_string())
//        addInstruction(Parser::SUB, val2);
    }
    else if(opp == MathParser::INDIRECTION)
    {
//        addInstruction(Parser::LOAD, val2, Parser::INT, true);
//        addInstruction(Parser::SAVE, tmp);
//        addInstruction(Parser::NOT, tmp);

        tmp = "*" + tmp;
    }
    else
        throw std::runtime_error("OPP not handled: " + opp.str);

    return tmp;
}

void Compiler::addFlag(std::string name, int line, Symbol::Type type)
{
    if(type == Symbol::UNDEFINED)
        type = isNumber(name) ? Symbol::CONST : Symbol::VAR;

    if(!mDatas.symbols[name])
        mDatas.symbols[name] = Symbol(type, name);

    mDatas.symbols[name].flagLines.push_back(line);
}

void Compiler::addSymbole(std::string name, Symbol::Type type)
{
    addSymbole(name, -1, type);
}

void Compiler::addSymbole(std::string name, int addr, Symbol::Type type)
{
    if(type == Symbol::UNDEFINED)
        type = isNumber(name) ? Symbol::CONST : Symbol::VAR;

    if(!mDatas.symbols[name])
        mDatas.symbols[name] = Symbol(type, name);

    mDatas.symbols[name].addr = addr;
}

void Compiler::addConst(std::string name)
{
    Variable v(name, INT_T);
    if(!mDatas.symbols[v.symbolName])
    {
        mVariables[v.name] = v;
        mDatas.symbols[v.symbolName] = v;
        addSymbole(name, Symbol::CONST);
    }

}

void Compiler::addVariable(std::string name, VarType type)
{
    Variable v(name, type);

    if(!!mDatas.symbols[v.symbolName])
        throw std::runtime_error("Variable \"" + name + "\" already defined");

    mVariables[v.name] = v;
    mDatas.symbols[v.symbolName] = v;
}

void Compiler::addFunction(std::string name, Function::FunctionSignature signature)
{
    Function v(name, signature);

    if(!!mDatas.symbols[v.symbolName])
        throw std::runtime_error("Function \"" + name + "\" already defined");

    mFunctions[v.name][v.signature] = v;
    mDatas.symbols[v.symbolName] = v;
}

Compiler::Symbol& Compiler::findSymbol(std::string name, bool throwIfNotFound)
{
    if(!!mDatas.symbols[name])
        return mDatas.symbols[name];
    else if(!!findVariable(name))
        return findVariable(name);
    else
    {
        for(auto& pair : findFunction(name))
        {
            if(!!pair.second)
                return pair.second;
        }
    }

    if(throwIfNotFound)
        throw std::runtime_error("Symbol \"" + name + "\" not defined");
    else
        return mDatas.symbols[""];
}

Compiler::Variable& Compiler::findVariable(std::string name)
{
    if(!!mVariables[name])
        return mVariables[name];

    return mVariables[""];
}

std::map<Compiler::Function::FunctionSignature, Compiler::Function>& Compiler::findFunction(std::string name)
{
//    if(!!mFunctions[name])
//        return mFunctions[name];

    return mFunctions[name];
}

Compiler::Function& Compiler::findFunction(std::string name, Function::FunctionSignature signature)
{
//    for(const Function& f : mFunctions[name])
//    {
//        if(f.signature == signature)
//            return f;
//    }
//
//    return mFunctions[""];

    return mFunctions[name][signature];
}

void Compiler::addInstruction(Parser::OPP opp, std::string flag, Parser::Type type, bool ptr)
{
    if(!flag.empty())
        addFlag(flag, mCurrentAddr);

    addInstruction(opp, 0, type, ptr);
}

void Compiler::addInstruction(Parser::OPP opp, int operand, Parser::Type type, bool ptr)
{
    mDatas.memory.resize(mDatas.memory.size() + 4);
//    mDatas.memory.at<uint32_t>(mCurrentAddr) = ((ptr << Parser::PTR_SHIFT) + (opp << Parser::OPP_SHIFT) + (type << Parser::TYPE_SHIFT) + (operand & Parser::OPERAND_MASK));
    mDatas.memory.at<uint32_t>(mCurrentAddr) = ((ptr * Parser::PTR_MASK) + (opp << Parser::OPP_SHIFT) + (type << Parser::TYPE_SHIFT) + (operand & Parser::OPERAND_MASK));
    mCurrentAddr += 4;
}

Compiler::Keyword Compiler::toKeyword(const std::string& string)
{
    static const std::vector<std::string> keywords =
    {
        "function", "if", "else", "while",
        "return", "end",
        "int",
        "extern"
    };

    assert(("You added/removed a Keyword but forgot to o the same for keywords strings", keywords.size() == Keyword::UNKNOW)); //make sure we dont forget to add a key word

//    for(int x = 0; x < Keywords.size(); x++)
//    {
//        if(((string.size() - pos) >= Keywords[x].size()) && string.substr(pos, Keywords[x].size()) == Keywords[x])
//            return (Keyword)x;
//    }

    for(size_t x = 0; x < keywords.size(); x++)
    {
        if(string == keywords[x])
            return (Keyword)x;
    }

    return UNKNOW;
}

bool Compiler::isScopedKeyword(const std::string& key)
{
    return isScopedKeyword(toKeyword(key));
}

bool Compiler::isScopedKeyword(Keyword key)
{
    return key == FUNCTION || key == WHILE || key == IF || key == ELSE;
}

bool Compiler::isTypeKeyword(const std::string& key)
{
    return isTypeKeyword(toKeyword(key));
}

bool Compiler::isTypeKeyword(Keyword key)
{
    return key == INT;
}

Compiler::VarType Compiler::toType(const std::string& key)
{
    return toType(toKeyword(key));
}

Compiler::VarType Compiler::toType(Keyword key)
{
    if(key == INT)
        return INT_T;

    return UNKNOW_T;
}

void Compiler::loadFromFile(std::string file)
{
    std::ifstream is(file);
    mProgram.assign(std::istreambuf_iterator<char>(is), std::istreambuf_iterator<char>());
//    is >> mProgram;
//    std::vector<std::string> program;
//    std::string line;

//    while( std::getline( is, line ) ) program.push_back( line);
//    while( std::getline( is, line ) ) mProgram.append(line);

//    loadFromVector(program);
}

void Compiler::loadFromVector(std::vector<std::string>& vec)
{
//    mProgram.clear();
//    mProgram = vec;
}

void Compiler::loadFromString(std::string program, std::string separator)
{
//    std::vector<std::string> vec(split(program, separator));
//    loadFromVector(vec);
}
