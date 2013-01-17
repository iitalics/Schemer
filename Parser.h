#pragma once
#include "Value.h"
#include <fstream>

class Token; class NumberToken; class ExpressionToken; class VariableToken;


class TokenFactory
{
public:
	TokenFactory ();
	~TokenFactory ();
	
	NumberToken* CreateNumberToken (Number n);
	NumberToken* CreateNumberToken (const std::string& s);
	VariableToken* CreateVariableToken (std::string s);
	ExpressionToken* CreateExpressionToken (std::vector<Token*> tokens);
	
private:
	std::vector<NumberToken*> numbers;
	std::vector<VariableToken*> variables;
	std::vector<Token*> others;
};

class Parser
{
public:
	Parser ();
	~Parser();
	
	TokenFactory* GetFactory ();
	TokenFactory* ObtainFactory ();
	
	void Parse (std::string data);
	void Parse (const std::stringstream& data);
	void Parse (std::fstream& data);
	
	std::vector<Token*> FlushTokens ();
private:
	
	void parse ();
	Token* parseToken ();
	
	char first ();
	char next ();
	bool eof ();
	void trim ();
	std::string block ();
	
	TokenFactory* _factory;
	std::string buffer;
	std::vector<Token*> parsed;
};
