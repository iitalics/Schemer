#include "includes.h"
#include "Token.h"




NumberToken::NumberToken (Number n) : Value(n)
{
	Type = TokenTypeNumber;
}

VariableToken::VariableToken (std::string name) : Name (name)
{
	Type = TokenTypeVariable;
}


ExpressionToken::ExpressionToken (Token* f, std::vector<Token*> args)
	: Function (f), Arguments (args)
{
	Type = TokenTypeExpression;
}

/* ---
 * |  factory handles deleting ALL tokens
 * ---
ExpressionToken::~ExpressionToken ()
{
	for (auto i = Arguments.begin(); i != Arguments.end(); ++i)
		delete *i;
}
* */

static std::string name (Token* t)
{
	if (t->Type == TokenTypeVariable)
		return ((VariableToken*)t)->Name;
	if (t->Type == TokenTypeNumber)
	{
		std::stringstream ss;
		ss << ((NumberToken*)t)->Value;
		return ss.str();
	}
	if (t->Type == TokenTypeExpression)
		return "{expression}";
	return "{nil}";
}

std::vector<std::string> ExpressionToken::Names (bool includeName)
{
	std::vector<std::string> result;
	result.resize(Arguments.size() + (includeName ? 1 : 0));
	
	int k = 0;
	
	if (includeName)
		result[k++] = name(Function);
	
	for (auto i = Arguments.begin(); i != Arguments.end(); i++)
		result[k++] = name(*i);
	return result;
}
bool ExpressionToken::IsDefine ()
{
	return Function->Type == TokenTypeVariable &&
			(((VariableToken*)Function)->Name == "define");
}


void displayToken (Token* t, int indent)
{
	for (int i = 0; i < indent; i++)
		std::cout << ' ';
	
	if (t->Type == TokenTypeNumber)
	{
		std::cout << "\x1b[34m" << ((NumberToken*)t)->Value << "\x1b[0m" << std::endl;
	}
	else if (t->Type == TokenTypeVariable)
	{
		std::cout << "\x1b[33m" << ((VariableToken*)t)->Name << "\x1b[0m" << std::endl;
	}
	else if (t->Type == TokenTypeExpression)
	{
		ExpressionToken* e = (ExpressionToken*)t;
		std::cout << "(";
		displayToken(e->Function, 0);
		
		
		for (auto i = e->Arguments.begin(); i != e->Arguments.end(); ++i)
			displayToken(*i, indent + 4);
			
		for (int i = 0; i < indent; i++)
			std::cout << ' ';
		
		std::cout << ")" << std::endl;
	}
	else std::cout << "(?)" << std::endl;
}
