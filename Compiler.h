#ifndef CLASS_H
#define CLASS_H

#include "stdafx.h"

#include "MathParser.h"
#include "Parser.h"
#include "Memory.h"
//////////////////////
///FEATURE IDEAS
/// yield
/// a,b = b,a;
///
//////////////////////

class Linker;

class Compiler
{
        struct Symbol;

    public:
        friend Linker;

        Compiler();

        struct CompiledData;

        CompiledData compile();

        void loadFromFile(std::string file);
        void loadFromVector(std::vector<std::string>& vec);
        void loadFromString(std::string program, std::string separator);

        struct CompiledData
        {
            Memory memory;
            std::map<std::string,Symbol> symbols;
        };

    private:

        enum VarType {INT_T, VOID_T, UNKNOW_T};

        struct Symbol
        {
            enum Type {VAR, CONST, FUNC, LABEL, UNDEFINED};

            Symbol() {};
            Symbol(Type type, std::string name) : isDef(true)
            {
                this->type = type;
                this->name = name;
            }

            void eval(std::map<std::string, Symbol>& symbols)
            {
                if(name.size() > 0 && name[0] == '$')
                {
                    name.erase(0, 1);
                    for(const int& line : flagLines)
                    {
                        std::string result = replaceAll(name, "CURRENT_ADDR", std::to_string(line));
                        int num = MathParser::solve(MathParser::toPostfix(result));
                        result = std::to_string(num);

                        if(!symbols[result])
                            symbols[result] = Symbol(CONST, result);

                        symbols[result].flagLines.push_back(line);
                    }

                    addr = 0;
                    flagLines.clear();
                    isDef = false;

                }
                else if(name == "CURRENT_ADDR")
                {
                    for(const int& line : flagLines)
                    {
                        std::string result = "CURRENT_ADDR_" + std::to_string(line);
                        if(!symbols[result])
                            symbols[result] = Symbol(CONST, result);

                        symbols[result].flagLines.push_back(line);
                        symbols[result].addr = line;
                    }

                    addr = 0;
                    flagLines.clear();
                    isDef = false;
                }
            }

            Type type = UNDEFINED;
            int addr = -1;

            std::string name = "";

            bool isDef = false;

            std::vector<int> flagLines;

            bool operator!() const
            {
                return !isDef;
            }

            void operator+=(const Symbol& other)
            {
                isDef = isDef || other.isDef;
                name = (name.size() ? name : other.name);
                type = (type == UNDEFINED ? other.type : type);
                addr = (addr == -1 ? other.addr : addr);

                for(const int& line : other.flagLines)
                    flagLines.push_back(line);
            }

            void operator+=(int offset)
            {
                if(addr != -1)
                    addr += offset;

                for(int& line :flagLines)
                    line += offset;
            }
        };

        struct Variable : Symbol
        {
            Variable(std::string name, VarType type) : variableType(type), symbolName("VARIABLE_" + name), Symbol(VAR, name)
            {
            }

            Variable() {}

            std::string symbolName;
            VarType variableType;
        };

        struct Function : Symbol
        {
            typedef std::vector<Variable> Params;

            struct FunctionSignature
            {
                Params params;

                bool operator==(const FunctionSignature& other) const
                {
                    if(params.size() != other.params.size())
                        return false;

                    for(int x = 0; x < params.size(); x++)
                    {
                        if(params[x].variableType != other.params[x].variableType)
                            return false;
                    }

                    return true;
                }

                bool operator<(const FunctionSignature& other) const
                {
                    return params.size() < other.params.size();
                }
            };

            Function(std::string name, FunctionSignature signature) : Symbol(FUNC, name), signature(signature)
            {
                symbolName = "FUNCTION_" + name;
                for(const Variable& v : signature.params)
                    symbolName += "_" + v.type;
            }

            Function() {}

            std::string symbolName;

            FunctionSignature signature;

            bool implemented = false;

            bool isExtern = false;
            std::string externName = "";
            uint32_t externHash = 0;
        };

        struct Context
        {
                friend Compiler;

                enum Type {CONDITION, WHILE, FUNCTION};

                Context(Type type, int id)
                {
                    this->type = type;
                    contextID = id;
                }

            private:

                int contextID;
                int nextPart = 0;
                Type type;

                std::string functionName = "";
        };

        void preParse();
        void preProcessor();

        void pushContext(Context::Type type);
        void popContext();
        std::string getContext(int id = -1);
        std::string getContextPart(int part);
        std::string getContextEnd();
        std::string addContextPart(int addr);

        std::string getFunctionReturn();
        std::string getFunctionEnd();

        void addFlag(std::string name, int line, Symbol::Type type = Symbol::UNDEFINED);

        void addSymbole(std::string name, Symbol::Type type = Symbol::UNDEFINED);
        void addSymbole(std::string name, int addr, Symbol::Type type = Symbol::UNDEFINED);

        void addConst(std::string name);
        void addVariable(std::string name, VarType type);
        void addFunction(std::string name, Function::FunctionSignature signature);

        Symbol& findSymbol(std::string name, bool throwIfNotFound = true);
        Variable& findVariable(std::string name);
        std::map<Function::FunctionSignature, Function>& findFunction(std::string name);
        Function& findFunction(std::string name, Function::FunctionSignature signature);

        void addInstruction(Parser::OPP opp, std::string flag = "", Parser::Type type = Parser::INT, bool ptr = false);
        void addInstruction(Parser::OPP opp, int operand, Parser::Type type = Parser::INT, bool ptr = false);

        std::string solve(std::string postfix);
        std::string solveOPP(MathParser::Operator opp, std::string val1, std::string val2);

        std::string mProgram;

        int mCurrentContext = 0;
        std::stack<Context> mContext;
        Function mCurrentFunction;

        std::stack<Function> mSolverFunctions;

        std::map<std::string, Variable> mVariables;
        std::map<std::string, std::map<Function::FunctionSignature, Function>> mFunctions;

//    std::map<std::string,Symbol> mSymbols;
//    Memory mMemory;
        CompiledData mDatas;

        int mCurrentAddr;
        int mCurrentSolverTemp;

        int mCurrentStringConst = 0;
        std::map<int, std::string> mStringConst;

        enum Keyword {FUNCTION, IF, ELSE, WHILE,
                      RETURN,
                      END,
                      INT,
                      EXTERN,
                      UNKNOW //UNKNOW shall be the last
                     };

//    static Keyword toKeyword(const std::string& string, size_t pos);
        static Keyword toKeyword(const std::string& string);
        static bool isScopedKeyword(const std::string& key);
        static bool isScopedKeyword(Keyword key);
        static bool isTypeKeyword(const std::string& key);
        static bool isTypeKeyword(Keyword key);
        static VarType toType(const std::string& key);
        static VarType toType(Keyword key);
};

#endif // CLASS_H
