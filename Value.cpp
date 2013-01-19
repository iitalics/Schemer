#include "includes.h"
#include "Value.h"
#include "Token.h"
#include "Interpreter.h"


SValue::~SValue () {}


/* **************** NumberValue *************** */

NumberValue::NumberValue (Number n) : Value(n)
{
	Type = ValueTypeNumber;
}
SValue* NumberValue::Copy () { return new NumberValue(Value); }


/* **************** PairValue *************** */
PairValue::PairValue (SValue* head, SValue* tail) : Head(head), Tail(tail)
{
	Type = ValueTypePair;
}
PairValue::~PairValue ()
{
	delete Head; delete Tail;
}
SValue* PairValue::Copy () { return new PairValue(Head->Copy(), Tail->Copy()); }



/* **************** FunctionValue *************** */
NormalFunctionValue::NormalFunctionValue (ExpressionToken* args, Token* body)
	: Arguments(args), Body(body)
{
	fType = FunctionTypeNormal;
	Type = ValueTypeFunction;
}
NormalFunctionValue::NormalFunctionValue () { Type = ValueTypeFunction; }
SValue* NormalFunctionValue::Call (Interpreter* t, std::vector<SValue*>& args)
{
	Scope s;
	s.Bind(Arguments, args);
	
	return t->Evaluate(Body, &s);
}
SValue* NormalFunctionValue::Copy () { return new NormalFunctionValue(Arguments, Body); }


NativeFunctionValue::NativeFunctionValue (NativeFunctionHandler h)
	: Handler(h)
{
	fType = FunctionTypeNative;
	Type = ValueTypeFunction;
}
SValue* NativeFunctionValue::Copy () { return new NativeFunctionValue(Handler); }
SValue* NativeFunctionValue::Call (Interpreter* t, std::vector<SValue*>& args)
{
	if (Handler == NULL) return new NullValue();
	return Handler(args);
}




LambdaFunctionValue::LambdaFunctionValue (ExpressionToken* args, Token* body, Scope* s)
{
	Arguments = args;
	Body = body;
	
	Type = ValueTypeFunction;
	fType = FunctionTypeLambda;
	
	scope = new Scope();
	
	if (s != NULL)
	{
		scope->Bind(*s);
	}
}
LambdaFunctionValue::~LambdaFunctionValue ()
{
	delete scope;
}
SValue* LambdaFunctionValue::Call (Interpreter* n, std::vector<SValue*>& values)
{
	Scope newScope(*scope);
	newScope.Bind(Arguments, values, true);
	
	return n->Evaluate(Body, &newScope);
}
SValue* LambdaFunctionValue::Copy ()
{
	return new LambdaFunctionValue(Arguments, Body, scope);
}


std::string NormalFunctionValue::Name ()
{
	std::stringstream ss; 
	ss << "[Function " << ((VariableToken*)Arguments->Function)->Name << "]";
	return ss.str();
}
std::string NativeFunctionValue::Name () { return "[Native function]"; }
std::string LambdaFunctionValue::Name () { return "[Lambda function]"; }



/* **************** Others *************** */
SValue* NullValue::Copy () { return new NullValue(); }

BooleanValue::BooleanValue (bool b) : Value(b)
{
	Type = ValueTypeBoolean;
}
SValue* BooleanValue::Copy () { return new BooleanValue(Value); }













SValue* SValue::Create (Number n)
{
	return new NumberValue(n);
}
SValue* SValue::Create (bool b)
{
	return new BooleanValue(b);
}
SValue* SValue::Create (SValue* a, SValue* b)
{
	return new PairValue(a, b);
}



static std::string NumberToString (Number n, int digits)
{
	std::stringstream ss;
	
	bool negative = n < 0;
	
	if (negative)
	{
		n = -n;
		ss << '-';
	}
	
	int i = 0;
	int base = (int)n;
	ss << base;
	n -= base;
	
	if (n > 0)
	{
		ss << '.';
		while (n > 0 && (++i) < digits)
		{
			n *= 10;
			base = (int)n;
			n -= base;
			ss << base;
		}
		
		std::string result(ss.str());
		
		int len = result.size();
		while (result[len - 1] == '0')
			len--;
		return result.substr(0, len);
	}
	
	return ss.str();
}





std::string SValue::String ()
{
	switch (Type)
	{
		case ValueTypeNull:
			return "nil";
		
		case ValueTypeNumber:
			return NumberToString(((NumberValue*)this)->Value, 16);
		
		case ValueTypeFunction:
			return ((FunctionValue*)this)->Name();
		
		case ValueTypePair:
		{
			PairValue* p = (PairValue*)this;
			std::stringstream ss;
			
			
			ss << "(" << p->Head->String();
			
			if (p->Tail->Type == ValueTypePair)
			{
				while (p->Tail->Type == ValueTypePair)
				{
					p = (PairValue*)p->Tail;
					ss << ", " << p->Head->String();
				}
				if (p->Tail->Type != ValueTypeNull)
					ss << p->Tail->String();
			}
			else
			{
				if (p->Tail->Type != ValueTypeNull)
					ss << " . " << p->Tail->String();
			}
			ss << ")";
			
			
			return ss.str();
		}
		
		case ValueTypeBoolean:
			return (((BooleanValue*)this)->Value) ? "#t" : "#f";
		
		default: break;
	}
	return "??";
}
