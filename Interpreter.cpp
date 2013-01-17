#include "includes.h"
#include "Interpreter.h"
#include "Parser.h"
#include "Token.h"





Interpreter::Interpreter ()
{
	globalScope = new Scope();
	RegisterNativeFunctions(globalScope);
	
}

Interpreter::~Interpreter ()
{
	for (auto i = factories.begin(); i != factories.end(); ++i)
		delete *i;
}
bool Interpreter::operator >> (std::string& s)
{
	if (output.empty()) return false;
	
	s = output.front();
	output.pop();
	return true;
}

void Interpreter::LoadParser (Parser& p)
{
	std::vector<Token*> parsed = p.FlushTokens();
	
	for (auto i = parsed.begin(); i != parsed.end(); ++i)
		tokens.push(*i);
		
	//TokenFactory* factory = p.GetFactory();
}


SValue* Interpreter::Evaluate (Token* t, Scope* scope, bool requireOutput)
{
	switch (t->Type)
	{
		case TokenTypeNumber:
			return new NumberValue(((NumberToken*)t)->Value);
		
		case TokenTypeVariable:
		{
			std::string name(((VariableToken*)t)->Name);
			
			if (name == "true") return new BooleanValue(true);
			if (name == "false") return new BooleanValue(false);
			if (name == "nil") return new NullValue();
			
			if (scope != NULL && scope->Contains(name))
				return scope->Get(name);
			
			return globalScope->Get(name);
		}
		
		case TokenTypeExpression:
		{
			ExpressionToken* e = (ExpressionToken*)t;
			
			std::string firstName("");
			
			if (e->Function->Type == TokenTypeVariable)
			{
				firstName = ((VariableToken*)(e->Function))->Name;
			}
			
			if (firstName == "define")
			{
				SValue* result;
				
				if (e->Arguments.size() != 2)
					die("Invalid define syntax");
				
				Token* args_token = e->Arguments[0];
				Token* body = e->Arguments[1];
				
				if (args_token->Type == TokenTypeVariable)
				{
					std::string name(((VariableToken*)args_token)->Name);
					globalScope->Set(name, result = Evaluate(body, scope));
				}
				else if (args_token->Type == TokenTypeExpression)
				{
					ExpressionToken* args = (ExpressionToken*)args_token;
					
					for (auto i = args->Arguments.begin(); i != args->Arguments.end(); i++)
						if ((*i)->Type != TokenTypeVariable)
							die("Invalid variable name");
					if (args->Function->Type != TokenTypeVariable)
						die("Invalid variable name");
					
					std::string name(((VariableToken*)args->Function)->Name);
					
					FunctionValue* function = new NormalFunctionValue(args, body);
					globalScope->Set(name, function);
					
					result = function;
				}
				if (requireOutput)
					return result;
			}
			else
			{
				FunctionValue* f = (FunctionValue*)Evaluate(e->Function, scope);
				if (f->Type != ValueTypeFunction)
					die(f->String() << " is not a valid function");
				
				std::vector<SValue*> args;
				args.resize(e->Arguments.size());
				
				for (unsigned int i = 0; i < e->Arguments.size(); i++)
					args[i] = Evaluate(e->Arguments[i], scope);
				
				SValue* result = f->Call(this, args);
				
				for (auto i = args.begin(); i != args.end(); ++i)
					delete *i;
				
				return result;
			}
			break;
		}
		
		default:
			if (requireOutput)
				return new NullValue();
	}
	return NULL;
}


void Interpreter::Run ()
{
	while (tokens.size() > 0)
	{
		Token* t = tokens.front();
		tokens.pop();
		
		SValue* value = Evaluate(t, NULL, false);
		
		if (value != NULL)
		{
			output.push(value->String());
			delete value;
		}
	}
}










Scope::Scope () {}
Scope::Scope (ExpressionToken* argl, std::vector<SValue*> args) { Bind(argl, args); }
Scope::Scope (const Scope& s) { Bind(s); }
Scope::~Scope ()
{
	for (auto i = data.begin(); i != data.end(); ++i)
		delete i->second;
}



void Scope::Bind (const Scope& s)
{
	for (auto i = s.data.begin(); i != s.data.end(); ++i)
		data[i->first] = i->second->Copy();
}
void Scope::Bind (ExpressionToken* e, std::vector<SValue*> args)
{
	std::vector<std::string> names = e->Names(false);
	unsigned int len = names.size();
	if (len > args.size())
		len = args.size();
	
	for (unsigned int i = 0; i < len; i++)
		Set(names[i], args[i]);
}

SValue* Scope::Get (std::string key)
{
	auto it = data.find(key);
	if (it == data.end())
		return new NullValue();
	return it->second->Copy();
}
void Scope::Set (std::string key, SValue* value)
{
	auto it = data.find(key);
	if (it != data.end())
	{
		delete it->second;
	}
	data[key] = value->Copy();
}
bool Scope::Contains (std::string key)
{
	return data.find(key) != data.end();
}
