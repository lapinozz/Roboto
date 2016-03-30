#include "stdafx.h"

#include "Parser.h"
#include "Compiler.h"
#include "Assembler.h"
#include "Linker.h"
#include "MathParser.h"
#include "Utility.h"

int main()
{
	seed(time(NULL));

	std::cout << (uint32_t)(int)murmurHash3_x86_32("size") << std::endl;
    std::cout << murmurHash3_x86_32("size") << std::endl;
    std::cout << murmurHash3_x86_32("superTest") << std::endl;


    std::string string;
    string = "1+-2";
    string = "1+(-2)";
    string = "1+-+-+-2";
    string = "1+-----2";
    string = "1+-(2)";
    string = "-2+1";
    string = "1+2+3+4+5";
//    string = "v=m*f";
    std::cout << string << std::endl;
    std::cout << MathParser::toPostfix(string) << std::endl;
    std::cout << MathParser::solve(MathParser::toPostfix(string)) << std::endl;

//    return 0;

    Parser parser;
    Compiler compiler;

    std::vector<Compiler::CompiledData> datas;

    compiler.loadFromFile("./code/code11-v2.roboto");
    datas.push_back(compiler.compile());
//    compiler.loadFromFile("./code/malloc.roboto");
//    datas.push_back(compiler.compile());

    parser.loadFromMemory(Linker::link(datas));
    parser.bindFunction("superTest", [](){std::cout << "working"; return 5;});
    parser.bindFunction("testSuper", [](int i){return 5*i;});
//    parser.bindFunction("size", [&](){return parser.getMemorySize();});
    parser.parse();

//    parser.bindFunction(0, [](){std::cout << "working";});
//    parser.bindFunction(1, [](int& i){std::cout << i;});
//    parser.bindFunction(2, [](int& i){i = 5;});
//    parser.bindFunction(3, [](int i, char c, float f){std::cout << i << ':' << c << ':' << (int)f;});
//    parser.bindFunction(3, [](int i, float x){return i*x;});
//    parser.bindFunction(4, [&](){return parser.getMemorySize();});
//    parser.loadFromMemory(Assembler::assemble(loadFile("./code/extern_call.SPASM")));
//    parser.parse();

//    std::cout << MathParser::solve(MathParser::toPostfix("(1 || 0) && 7 < 8 && 41--2 == 43 && (7-3)*2 == 8 && 0 == !1 && 1 == !0 && (!(!1&&0) && (!1 || 1))"));
//    std::cout << MathParser::toPostfix("getnum ( 7 ) + getnum ( 3 )") << std::endl;
    return EXIT_SUCCESS;
}

