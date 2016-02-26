#ifndef MATHPARSER_H
#define MATHPARSER_H

#include "stdafx.h"

class Compiler;

class MathParser
{
    friend Compiler;

public:
    static std::string toPostfix(std::string infix);
    static float solve(std::string postfix);

private:
    MathParser();

    enum OPP {ADD, SUB, MUL, DIV, EXP, MOD,
     UNDEFINED,
     OPEN_PARENTHESES, CLOSE_PARENTHESES, COMMA,
     FUNCTION_PARAM, FUNCTION_CALL,
     ASSIGNMENT,
     COMPARE_EQUALITY, COMPARE_LESS, COMPARE_GREATER, COMPARE_NOT_EQUALE, COMPARE_LESS_OR_EQUALE, COMPARE_GREATER_OR_EQUALE,
     NOT, OR, AND,
     COMMENT, LINE_SEPARATOR,
     SCOPE_START, SCOPE_END};

    static float solveOPP(OPP opp, float val1, float val2);

    static OPP toOPP(std::string opp);
    static OPP toOPP(char opp);
    static int hasPrecedence(OPP opp1, OPP opp2);
    static int getWeight(OPP opp);
};
#endif // MATHPARSER_H
