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
	//std::cout << "going to free " << factories.size() << " factories" << std::endl;
	for (auto i = factories.begin(); i != factories.end(); ++i)
	{
		delete *i;
	}
	
	delete globalScope;
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
		
	TokenFactory* factory = p.ObtainFactory();
	if (factory != NULL)
	{
		factories.push_back(factory);
	}
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
			
			if (name == "#t") return new BooleanValue(true);
			if (name == "#f") return new BooleanValue(false);
			if (name == "nil") return new NullValue();
			
			if (scope != NULL && scope->Contains(name))
				return scope->Get(name);
			
			return globalScope->Get(name);
		}
		
		case TokenTypeString:
		{
			return new StringValue(((StringToken*)t)->Text);
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
				if (e->Arguments.size() != 2)
					die("Invalid define syntax");
				
				Token* args_token = e->Arguments[0];
				Token* body = e->Arguments[1];
				
				if (args_token->Type == TokenTypeVariable)
				{
					std::string name(((VariableToken*)args_token)->Name);
					globalScope->Set(name, Evaluate(body, scope), false);
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
					
					globalScope->Set(name, new NormalFunctionValue(args, body), false);
				}
				else die("Invalid define syntax");
				
				if (requireOutput)
					return new NullValue();
			}
			else if (firstName == "if")
			{
				if (e->Arguments.size() != 3)
					die("Invalid 'if' syntax, expected 3 arguments");
				
				SValue* condition = Evaluate(e->Arguments[0], scope, true);
				if (condition->Type != ValueTypeBoolean)
					die("Invalid 'if' statement, condition '" << condition->String() << "' is not boolean type");
				
				SValue* result;
				
				if (((BooleanValue*)condition)->Value)
					result = Evaluate(e->Arguments[1], scope, true);
				else
					result = Evaluate(e->Arguments[2], scope, true);
				
				delete condition;
				return result;
			}
			else if (firstName == "cond")
			{
				auto args = e->Arguments;
				for (auto i = args.cbegin(); i != args.cend(); ++i)
				{
					BooleanValue* cond = (BooleanValue*)Evaluate(*i, scope, true);
					
					if (cond->Type != ValueTypeBoolean)
						die("Invalid condition, '" << cond->String() << "' is not boolean type");
					
					bool cond_value = cond->Value;
					delete cond;
					
					if ((i + 1) == args.cend())
						die("Invalid condition at end of cond statement");
					
					Token* body = *(++i);
					
					if (cond_value)
						return Evaluate(body, scope, true);
				}
				if (!requireOutput)
					return NULL;
				return new NullValue();
			}
			else if (firstName == "lambda")
			{
				if (e->Arguments.size() != 2)
					die("Invalid lambda syntax");
				
				Token* args_token = e->Arguments[0];
				Token* body = e->Arguments[1];
				
				if (args_token->Type != TokenTypeExpression)
					die("Invalid lambda arguments");
				
				LambdaFunctionValue* lambda = new LambdaFunctionValue(
					(ExpressionToken*)args_token, body, scope);
				return lambda;
			}
			else if (firstName == "let")
			{
				if (e->Arguments.size() != 2 ||
				    e->Arguments[0]->Type != TokenTypeExpression)
					die("Invalid let syntax");
				ExpressionToken* vars_e = (ExpressionToken*)(e->Arguments[0]);
				Token* body = e->Arguments[1];
				
				Scope* newScope;
				if (scope == NULL)
					newScope = new Scope();
				else
					newScope = new Scope(*scope);
				
				std::vector<Token*> vars;
				vars.push_back(vars_e->Function);
				for (auto i = vars_e->Arguments.cbegin(); i != vars_e->Arguments.cend(); i++)
					vars.push_back(*i);
				for (auto i = vars.cbegin(); i != vars.cend(); i++)
				{
					ExpressionToken* v = (ExpressionToken*)*i;
					
					if (v->Type != TokenTypeExpression ||
					    v->Function->Type != TokenTypeVariable ||
					    v->Arguments.size() != 1)
					{
						die("Invalid let variable");
					}
					
					newScope->Set(((VariableToken*)v->Function)->Name,
						Evaluate(v->Arguments[0], newScope, true), false);
				}
				
				SValue* result = Evaluate(body, newScope);
				delete newScope;
				return result;
			}
			else if (firstName == "last" || firstName == "do")
			{
				if (e->Arguments.size() == 0)
				{
					if (requireOutput)
						return new NullValue();
					return NULL;
				}
				SValue* last = NULL;
				for (auto i = e->Arguments.begin(); i != e->Arguments.end(); i++)
				{
					delete last;
					last = Evaluate(*i, scope, true);
				}
				return last;
			}
			else if (firstName == "repeat")
			{
				if (e->Arguments.size() != 1)
					die("Expected only one argument");
				
				Token* token = e->Arguments[0];
				SValue* value = NULL;
				for (;;)
				{
					delete value;
					value = Evaluate(token, scope, true);
					if (value->Type == ValueTypeBoolean)
					{
						if (((BooleanValue*)value)->Value)
							break;
					}
					else if (value->Type != ValueTypeNull)
						break;
				}
				return value;
			}
			else
			{
				FunctionValue* f = (FunctionValue*)Evaluate(e->Function, scope);
				if (f->Type != ValueTypeFunction)
				{
					if (e->Function->Type == TokenTypeVariable)
						die("Cannot find function '" <<
						       ((VariableToken*)(e->Function))->Name << "'");
					else
						die(f->String() << " is not a valid function");
				}
				
				std::vector<SValue*> args;
				args.resize(e->Arguments.size());
				
				for (unsigned int i = 0; i < e->Arguments.size(); i++)
					args[i] = Evaluate(e->Arguments[i], scope);
				
				SValue* result = f->Call(this, args);
				
				for (auto i = args.begin(); i != args.end(); ++i)
					delete *i;
				
				delete f;
				
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

void Interpreter::findAllTokens (Token* root, std::vector<Token*>* list)
{
	list->push_back(root);
	if (root->Type == TokenTypeExpression)
	{
		ExpressionToken* e = (ExpressionToken*)root;
		
		findAllTokens(e->Function, list);
		for (auto i = e->Arguments.cbegin(); i != e->Arguments.cend(); i++)
			findAllTokens(*i, list);
	}
}


void Interpreter::GarbageCollect ()
{
	if (factories.empty()) return;
	
	for (auto i = factories.begin(); i != factories.end(); i++)
		(*i)->GarbageCollectBegin();
	
	
	std::vector<Token*> allTokens;
	std::vector<SValue*> allValues = globalScope->AllValues();
	for (auto s = allValues.cbegin(); s != allValues.cend(); s++)
	{
		//std::cout << "searching " << (*s)->String() << std::endl;
		if ((*s)->Type == ValueTypeFunction)
		{
			NormalFunctionValue* f = (NormalFunctionValue*)*s;
			if (f->fType & FunctionTypeNormal)
			{
				findAllTokens(f->Arguments, &allTokens);
				findAllTokens(f->Body, &allTokens);
			}
		}
	}
	allValues.clear();
	
	/*std::cout << "all tokens: " << std::endl;
	for (unsigned int i = 0; i < allTokens.size(); i++)
	{
		displayToken(allTokens[i]);
	}*/
	
	for (auto t = allTokens.begin(); t != allTokens.end(); t++)
	{
		for (auto i = factories.begin(); i != factories.end(); i++)
			(*i)->GarbageCollectProcess(*t);
	}
	
	for (auto i = factories.begin(); i != factories.end(); i++)
	{
		(*i)->GarbageCollectEnd();
		
		if ((*i)->Empty())
		{
			//std::cout << "ridding the whole of factory " << (*i)->name << std::endl;
			delete *i;
			factories.erase(i);
			i--;
		}
	}
}










Scope::Scope () {}
Scope::Scope (ExpressionToken* argl, std::vector<SValue*>& args, bool icn) { Bind(argl, args, icn); }
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
void Scope::Bind (ExpressionToken* e, std::vector<SValue*>& args, bool includeName)
{
	std::vector<std::string> names = e->Names(includeName);
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
void Scope::Set (std::string key, SValue* value, bool copy)
{
	auto it = data.find(key);
	if (it != data.end())
	{
		delete it->second;
	}
	if (copy)
		data[key] = value->Copy();
	else
		data[key] = value;
}
bool Scope::Contains (std::string key)
{
	return data.find(key) != data.end();
}
std::vector<SValue*> Scope::AllValues ()
{
	std::vector<SValue*> t;
	for (auto i = data.begin(); i != data.end(); i++)
		t.push_back(i->second);
	return t;
}
