#pragma once
#include "Value.h"
#include <fstream>

class Token; class NumberToken; class ExpressionToken; class VariableToken; class StringToken;


class TokenFactory
{
public:
	TokenFactory ();
	~TokenFactory ();
	
	NumberToken* CreateNumberToken (Number n);
	NumberToken* CreateNumberToken (const std::string& s);
	VariableToken* CreateVariableToken (std::string s);
	ExpressionToken* CreateExpressionToken (std::vector<Token*> tokens);
	StringToken* CreateStringToken (const std::string& s);
	
	bool Empty ();
	
	void GarbageCollectBegin ();
	void GarbageCollectEnd ();
	void GarbageCollectProcess (Token* t);
	
private:
	std::vector<NumberToken*> numbers;
	std::vector<VariableToken*> variables;
	std::vector<Token*> others;
	
	std::vector<Token*> gcLeft;
	std::vector<Token*> gcFound;
	bool contains (Token* t);

//	std::string name;
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
	
	void deleteComments ();
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
