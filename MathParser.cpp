#include "MathParser.h"

#include "Utility.h"

std::string MathParser::toPostfix(const std::string& infix)
{
    std::string postfix;
    for(size_t x = 0; x < infix.length(); x++)
    {
//        if(infix[x] == ' ')
//            continue;

        Operator opp;
        for(size_t size = MAX_OPP_SIZE; size > 0; size--)
        {
            opp = toOPP(infix.substr(x, size));
            if(opp != UNDEFINED)
                break;
        }

        if(opp != UNDEFINED)
        {
            postfix += " ";
            postfix += opp.str;
            postfix += " ";

            x += (opp.str.size() ? opp.str.size() - 1 : 0);
        }
        else
            postfix += infix[x];
    }

    removeExtraSpace(postfix);
//    std::cout << postfix << std::endl;

    std::stack<bool> insideFunctions;
    insideFunctions.push(false);

    std::vector<std::string> tokens;
    std::vector<std::string> symbols = split(postfix, " ");

    for(size_t x = 0; x < symbols.size(); x++)
    {
        auto prevOpp = x == 0 ? toOPP(ADD) : toOPP(symbols[x - 1]); //previous opp
        auto nextOpp = x == symbols.size() - 1 ? toOPP(ADD) : toOPP(symbols[x + 1]);  //current opp

        Unary unary = ANY;
        if(prevOpp != UNDEFINED && prevOpp != CLOSE_PARENTHESES)
            unary = LEFT;
//        else if(nextOpp != UNDEFINED && nextOpp != OPEN_PARENTHESES)
//            unary = RIGHT;
        else
            unary = BINARY;

        auto currOpp = toOPP(symbols[x], unary);

        if(currOpp != UNDEFINED)
        {
            if(insideFunctions.top() && currOpp == COMMA)
                currOpp = toOPP(FUNCTION_PARAM);

            tokens.push_back(currOpp.encodedStr);

            if(currOpp == OPEN_PARENTHESES && prevOpp != UNDEFINED)
            {
                insideFunctions.push(false);
            }
            else if(currOpp == OPEN_PARENTHESES && prevOpp == UNDEFINED)
            {
                insideFunctions.push(true);
                std::swap(tokens[tokens.size() - 1], tokens[tokens.size() - 2]);

                if(nextOpp != CLOSE_PARENTHESES)
                    tokens.push_back(toOPP(FUNCTION_PARAM).encodedStr);
            }
            else if(currOpp == CLOSE_PARENTHESES)
            {
                if(insideFunctions.top())
                {
                    tokens.emplace_back("0");
                    tokens.push_back(toOPP(FUNCTION_CALL).encodedStr);

                    std::swap(tokens[tokens.size() - 1], tokens[tokens.size() - 3]);
                }

                insideFunctions.pop();
            }
        }
        else
            tokens.push_back(symbols[x]);
    }

    for(const auto& token : tokens)
        std::cout << (toOPP(token) != UNDEFINED ? toOPP(token).str : token) << " ";
    std::cout << std::endl;

    std::stack<std::string> s;
    postfix.clear();

    for(auto& token : tokens)
    {
        auto opp = toOPP(token);
        if(opp != UNDEFINED)
        {
            if(opp == OPEN_PARENTHESES)
            {
                s.push(token);
            }
            else if(opp == CLOSE_PARENTHESES)
            {
                while(!s.empty() && toOPP(s.top()) != OPEN_PARENTHESES)
                {
                    postfix += s.top() + " ";
                    s.pop();
                }
                s.pop();
            }
            else
            {
                while(!s.empty() && toOPP(s.top()) != OPEN_PARENTHESES && toOPP(s.top()) < opp)
                {
                    postfix += s.top() + " ";
                    s.pop();
                }
                s.push(token);
            }
        }
        else
            postfix += token + " ";
    }

    while(!s.empty())
    {
        postfix += s.top() + " ";
        s.pop();
    }

    for(const auto& token : split(postfix, " "))
        std::cout << (toOPP(token) != UNDEFINED ? toOPP(token).str : token) << " ";
    std::cout << std::endl;


    return postfix;
}

float MathParser::solve(std::string postfix)
{
    removeExtraSpace(postfix);

    std::vector<std::string> tokens(split(postfix, " "));
    std::stack<std::string> s;
    for(std::string& token : tokens)
    {
        auto opp = toOPP(token);
        if(opp != UNDEFINED)
        {
            float x = std::stof(s.top());
            s.pop();

            float y = 0.f;
            if(opp.unary == BINARY)
            {
                y = std::stof(s.top());
                s.pop();
            }

            s.push(std::to_string(solveOPP(opp, y, x)));
        }
        else
            s.push(token);
    }

    return std::stod(s.top());
}

float MathParser::solveOPP(OPP opp, float val1, float val2)
{
    if(opp == MathParser::ADD)
        return val1 + val2;
    else if(opp == SUB)
        return val1 - val2;
    else if(opp == MUL)
        return val1 * val2;
    else if(opp == DIV)
        return val1 / val2;
    else if (opp == EXP)
        return pow(val1, val2);
    else if (opp == MOD)
        return (int)val1 % (int)val2; //I don't accept this, but I can't fix it either, so your job.
    else if(opp == OR)
        return val1 || val2;
    else if(opp == AND)
        return val1 && val2;
    else if(opp == COMPARE_EQUALITY)
        return val1 == val2;
    else if(opp == COMPARE_NOT_EQUALE)
        return val1 != val2;
    else if(opp == COMPARE_LESS)
        return val1 < val2;
    else if(opp == COMPARE_GREATER)
        return val1 > val2;
    else if(opp == COMPARE_LESS_OR_EQUALE)
        return val1 <= val2;
    else if(opp == COMPARE_GREATER_OR_EQUALE)
        return val1 >= val2;

    ///TODO: for unary operator the value to use is val2, not so good, might make a second function to be sure to have no confusion

    else if(opp == UNARY_MINUS)
        return -val2;
    else if(opp == UNARY_PLUS)
        return +val2;
    else if(opp == NOT)
        return !val2;
    else
    {
        throw "You retard used a non-existing operator exception";
    }
}

MathParser::Operator MathParser::toOPP(std::string strOpp, Unary unary)
{
    static_assert(sizeof(operators) / sizeof(Operator) == (size_t)COUNT, "Number of definition should be equal to number of declaration minus one");

    if(strOpp.size() >= 2 && strOpp[0] == OPP_IDENTIFIER)
        return toOPP((OPP)strOpp[1]);

    for(const auto& opp : operators)
    {
        if((unary == ANY || opp.unary == unary || opp.unary == ANY) && opp == strOpp)
            return opp;
    }

    return {};
}

MathParser::Operator MathParser::toOPP(OPP opp)
{
    static_assert(sizeof(operators) / sizeof(Operator) == (size_t)COUNT, "Number of definition should be equal to number of declaration minus one");

    for(const auto& o : operators)
    {
        if(o == opp)
            return o;
    }

    return {};
}
