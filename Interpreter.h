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
	
	void GarbageCollect ();
	
private:
	std::vector<TokenFactory*> factories;
	std::queue<Token*> tokens;
	std::queue<std::string> output;
	
	Scope* globalScope;
	
	void findAllTokens (Token* root, std::vector<Token*>* list);
};

class Scope
{
public:
	Scope ();
	Scope (ExpressionToken* argl, std::vector<SValue*>& args, bool includeName = false);
	Scope (const Scope& s);
	~Scope ();
	
	
	void Bind (ExpressionToken* argl, std::vector<SValue*>& args, bool includeName = false);
	void Bind (const Scope& s);
	
	
	SValue* Get (std::string key);
	void Set (std::string key, SValue* value, bool copy = true);
	bool Contains (std::string key);
	
	std::vector<SValue*> AllValues ();
	
private:
	std::map<std::string,SValue*> data;
};




void RegisterNativeFunctions (Scope*);
