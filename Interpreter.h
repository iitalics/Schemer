#pragma once
#include "Value.h"
#include "Token.h"
#include <queue>

class Parser; class TokenFactory; class Scope;


class Interpreter
{
public:
	Interpreter ();
	~Interpreter ();
	
	
	void LoadParser (Parser& p);
	void Run ();
	
	SValue* Evaluate (Token* t, Scope* s, bool requireOutput = true);
	
	bool operator >> (std::string& line);
	
private:
	std::vector<TokenFactory*> factories;
	std::queue<Token*> tokens;
	std::queue<std::string> output;
	
	Scope* globalScope;
};

class Scope
{
public:
	Scope ();
	Scope (ExpressionToken* argl, std::vector<SValue*> args);
	Scope (const Scope& s);
	~Scope ();
	
	
	void Bind (ExpressionToken* argl, std::vector<SValue*> args);
	void Bind (const Scope& s);
	
	
	SValue* Get (std::string key);
	void Set (std::string key, SValue* value);
	bool Contains (std::string key);
private:
	std::map<std::string,SValue*> data;
};




void RegisterNativeFunctions (Scope*);
