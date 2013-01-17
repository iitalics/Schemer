#pragma once
#include "Value.h"

enum TokenType
{
	TokenTypeNumber = 1,
	TokenTypeVariable = 2,
	TokenTypeExpression = 3,
	TokenTypeNull = 0
};

class Token
{
public:
	TokenType Type;
};


class NumberToken : public Token
{
public:
	NumberToken (Number n);
	Number Value;
};

class VariableToken : public Token
{
public:
	VariableToken (std::string name);
	std::string Name;
};

class ExpressionToken : public Token
{
public:
	ExpressionToken (Token* f, std::vector<Token*> args);
	
	Token* Function;
	std::vector<Token*> Arguments;
	
	std::vector<std::string> Names (bool includeName = true);
	bool IsDefine ();
};


void displayToken (Token* t, int indent = 0);
