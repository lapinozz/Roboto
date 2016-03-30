#ifndef MATHPARSER_H
#define MATHPARSER_H

#include "stdafx.h"

namespace MathParser
{
    std::string toPostfix(const std::string& infix);
    float solve(std::string postfix);

    const int MAX_OPP_SIZE = 2;
    const char OPP_IDENTIFIER = '\001';

    enum OPP : uint8_t
    {
        ADD, SUB, MUL, DIV, EXP, MOD,
        UNARY_MINUS, UNARY_PLUS,
        OPEN_PARENTHESES, CLOSE_PARENTHESES, COMMA,
        FUNCTION_PARAM, FUNCTION_CALL,
        ASSIGNMENT,
        COMPARE_EQUALITY, COMPARE_LESS, COMPARE_GREATER, COMPARE_NOT_EQUALE, COMPARE_LESS_OR_EQUALE, COMPARE_GREATER_OR_EQUALE,
        NOT, OR, AND, XOR,
        INDIRECTION, REFERENCE,
        COMMENT,
        LINE_END,
        OPEN_BRACKET, CLOSE_BRACKET,

        UNDEFINED,
        COUNT
    };

    struct Operator;

    enum Unary
    {
        ANY, LEFT, RIGHT, BINARY
    };

    enum Associativity
    {
        LEFT_TO_RIGHT, RIGHT_TO_LEFT
    };

    float solveOPP(OPP opp, float val1, float val2 = 0.f);

    Operator toOPP(std::string opp, Unary unary = ANY);
//    Operator toOPP(char opp, Unary unary = ANY);
    Operator toOPP(OPP opp);

    struct Operator
    {
        OPP opp = UNDEFINED;
        std::string str = "";

        size_t precedence = -1;

        Unary unary = BINARY;
        Associativity associativity = LEFT_TO_RIGHT;

        std::string encodedStr = std::string(1, OPP_IDENTIFIER) + (char)opp;

        operator OPP() const
        {
            return opp;
        }

        void operator=(const OPP& opp)
        {
            *this = toOPP(opp);
        }

        void operator=(const std::string& strRepresentation)
        {
            *this = toOPP(strRepresentation);
        }

        bool operator==(const OPP& opp) const
        {
            return this->opp == opp;
        }

        bool operator==(const std::string& strRepresentation) const
        {
            return str == strRepresentation;
        }

        bool operator!=(const OPP& opp) const
        {
            return this->opp != opp;
        }

        bool operator!=(const std::string& strRepresentation) const
        {
            return str != strRepresentation;
        }

        bool operator<(const Operator& other) const // do this as more precedence than other
        {
            if(associativity == LEFT_TO_RIGHT)
                return precedence <= other.precedence;
            else
                return precedence < other.precedence;
        }
    };

//    const static std::array<Operator, OPP::COUNT> operators = {{
    static Operator operators[OPP::COUNT] =
    {
        {NOT,         "!", 3, LEFT, RIGHT_TO_LEFT},
        {INDIRECTION, "*", 3, LEFT, RIGHT_TO_LEFT},
        {REFERENCE,   "&", 3, LEFT, RIGHT_TO_LEFT},
        {UNARY_MINUS, "-", 3, LEFT, RIGHT_TO_LEFT},
        {UNARY_PLUS,  "+", 3, LEFT, RIGHT_TO_LEFT},

        {MUL, "*", 5},
        {DIV, "/", 5},
        {MOD, "%", 5},

        {ADD, "+", 6},
        {SUB, "-", 6},

        {COMPARE_LESS, "<", 8},
        {COMPARE_GREATER, ">", 8},
        {COMPARE_LESS_OR_EQUALE, "<=", 8},
        {COMPARE_GREATER_OR_EQUALE, ">=", 8},

        {COMPARE_EQUALITY, "==", 9},
        {COMPARE_NOT_EQUALE, "!=", 9},

        {XOR, "^", 11},

        {AND, "&&", 13},

        {OR, "||", 14},

        {ASSIGNMENT, "=", 15},

        {COMMA, ",", 16},

        {OPEN_PARENTHESES,  "(", -1u, ANY},
        {CLOSE_PARENTHESES, ")", -1u, ANY},
        {FUNCTION_PARAM, "#"},
        {FUNCTION_CALL, "@"},
        {COMMENT, "//"},
        {LINE_END, ";"},

        {OPEN_BRACKET, "{"},
        {CLOSE_BRACKET, "}"},

        {UNDEFINED, ""}
    };


//    const std::vector<Operator>& operators()
//    {
//        const static std::vector<Operator> operators =
//        {
//            {ADD, "+"}
//        };
//
//        static_assert(operators.size())
//        return operators;
//    }





};

#endif // MATHPARSER_H
