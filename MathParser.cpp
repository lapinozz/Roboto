#include "MathParser.h"

#include "Utility.h"

MathParser::MathParser()
{
}

std::string MathParser::toPostfix(std::string infix)
{
    for(unsigned int x = 0; x < infix.length(); x++)
    {
        OPP opp = (x < infix.length() ? toOPP(infix.substr(x, 2)) : UNDEFINED);
        if(toOPP(infix[x]) != UNDEFINED || opp != UNDEFINED)
        {
            infix.insert(x + 1 + (opp == UNDEFINED ? 0 : 1) - (infix.length() - 1 == x ? 1 : 0), " ");
            infix.insert(x, " ");
            x += (opp == UNDEFINED ? 0 : 1);
            x++;
        }
    }

    removeExtraSpace(infix);

    std::vector<std::string> tokens(split(infix, " "));

    int lastWasOPP = true;
    for(unsigned int x = 0; x < tokens.size(); x++)
    {
        OPP opp = toOPP(tokens[x]);
        if(opp != UNDEFINED)
        {
            if(opp == COMMA)
            {
                tokens[x] = "#";
                opp = FUNCTION_PARAM;
            }

            if(lastWasOPP && opp == SUB)
            {
                tokens[x] = "-1";
                tokens.insert(tokens.begin() + x + 1, "*");
            }
            else if(lastWasOPP && opp == NOT)
            {
                tokens.insert(tokens.begin() + x, "0");
                x++;
            }
            else if((!lastWasOPP
                     && toOPP(tokens[x - 1]) != FUNCTION_PARAM) && opp == OPEN_PARENTHESES)
            {
                tokens.erase(tokens.begin() + x);
                tokens.insert(tokens.begin() + x - 1, "(");

                if(toOPP(tokens[x + 1]) != CLOSE_PARENTHESES)
                    tokens.insert(tokens.begin() + x + 1, "#");

                x++;
                int diff = 1;
                for(unsigned int y = x; y < tokens.size(); y++)
                {
                    opp = toOPP(tokens[y]);
                    if(opp == OPEN_PARENTHESES)
                        diff++;
                    else if(opp == CLOSE_PARENTHESES)
                    {
                        diff--;
                        if(diff == 0)
                        {
                            tokens.insert(tokens.begin() + y, "0");
                            tokens.insert(tokens.begin() + y, "@");
                            break;
                        }
                    }
                }

                opp = toOPP(tokens[x]);
            }
        }

        lastWasOPP = opp != UNDEFINED && opp != CLOSE_PARENTHESES;
    }

//    for(unsigned int x = 0; x < tokens.size(); x++)
//        std::cout << tokens[x] << " ";
//
//    std::cout << std::endl;

    std::stack<std::string> s;
    std::string postfix = "";

    for(std::string& token : tokens)
    {
        OPP opp = toOPP(token);
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
                while(!s.empty() && toOPP(s.top()) != OPEN_PARENTHESES && hasPrecedence(toOPP(s.top()), opp))
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

    return postfix;
}

float MathParser::solve(std::string postfix)
{
    removeExtraSpace(postfix);

    std::vector<std::string> tokens(split(postfix, " "));
    std::stack<std::string> s;
    for(std::string& token : tokens)
    {
        OPP opp = toOPP(token);
        if(opp != UNDEFINED)
        {
            float x = std::stof(s.top());
            s.pop();
            float y = std::stof(s.top());
            s.pop();
            s.push(std::to_string(solveOPP(opp, y, x)));
        }
        else
            s.push(token);
    }

    return std::stod(s.top());
}

float MathParser::solveOPP(OPP opp, float val1, float val2)
{
    if(opp == ADD)
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
    else if(opp == NOT)
        return !val2;
    else
    {
        throw "You retard used a non-existing operator exception";
    }
}

MathParser::OPP MathParser::toOPP(std::string opp)
{
    if(opp.length() == 2)
    {
        if(opp == "==")
            return COMPARE_EQUALITY;
        else if(opp == "!=")
            return COMPARE_NOT_EQUALE;
        else if(opp == "<=")
            return COMPARE_LESS_OR_EQUALE;
        else if(opp == ">=")
            return COMPARE_GREATER_OR_EQUALE;
        else if(opp == "||")
            return OR;
        else if(opp == "&&")
            return AND;
        else if(opp == "//")
            return COMMENT;
    }

    return opp.length() == 1 ? toOPP(opp[0]) : UNDEFINED;
}

MathParser::OPP MathParser::toOPP(char opp)
{
    if(opp == '+')
        return ADD;
    else if(opp == '-')
        return SUB;
    else if(opp == '*')
        return MUL;
    else if(opp == '/')
        return DIV;
    else if(opp == '^')
        return EXP;
    else if(opp == '%')
        return MOD;
    else if(opp == '(')
        return OPEN_PARENTHESES;
    else if(opp == ')')
        return CLOSE_PARENTHESES;
    else if(opp == ',')
        return COMMA;
    else if(opp == '#')
        return FUNCTION_PARAM;
    else if(opp == '@')
        return FUNCTION_CALL;
    else if(opp == '=')
        return ASSIGNMENT;
    else if(opp == '!')
        return NOT;
    else if(opp == '<')
        return COMPARE_LESS;
    else if(opp == '>')
        return COMPARE_GREATER;
    else if(opp == '{')
        return SCOPE_START;
    else if(opp == '}')
        return SCOPE_END;
    else if(opp == ';')
        return LINE_SEPARATOR;

    return UNDEFINED;
}

int MathParser::hasPrecedence(OPP opp1, OPP opp2)
{
    if(getWeight(opp1) == getWeight(opp2))
        return true;

    return getWeight(opp1) > getWeight(opp2);
}

int MathParser::getWeight(OPP opp)
{
    if(opp == OR)
        return 1;
    else if(opp == AND)
        return 2;
    else if(opp == COMPARE_EQUALITY || opp == COMPARE_NOT_EQUALE)
        return 3;
    else if(opp == COMPARE_LESS || opp == COMPARE_GREATER || opp == COMPARE_GREATER_OR_EQUALE || opp == COMPARE_LESS_OR_EQUALE)
        return 4;
    else if(opp == ADD || opp == SUB)
        return 5;
    else if(opp == MUL || opp == DIV || opp == MOD)
        return 6;
    else if(opp == EXP)
        return 7;
    else if(opp == NOT)
        return 8;

    return -1;
}
