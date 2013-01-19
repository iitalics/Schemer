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
NormalFunctionValue::NormalFunctionValue (ExpressionToken* args, Token* body) : Arguments(args), Body(body)
{
	Type = ValueTypeFunction;
}
SValue* NormalFunctionValue::Call (Interpreter* t, std::vector<SValue*>& args)
{
	Scope s;
	s.Bind(Arguments, args);
	
	return t->Evaluate(Body, &s);
}
SValue* NormalFunctionValue::Copy () { return new NormalFunctionValue(Arguments, Body); }


NativeFunctionValue::NativeFunctionValue (NativeFunctionHandler h) : Handler(h)
{
	Type = ValueTypeFunction;
}
SValue* NativeFunctionValue::Copy () { return new NativeFunctionValue(Handler); }
SValue* NativeFunctionValue::Call (Interpreter* t, std::vector<SValue*>& args)
{
	if (Handler == NULL) return new NullValue();
	return Handler(args);
}

std::string NormalFunctionValue::Name ()
{
	std::stringstream ss; 
	ss << "[Function " << ((VariableToken*)Arguments->Function)->Name << "]";
	return ss.str();
}
std::string NativeFunctionValue::Name ()
{
	return "[Native function]";
}




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
			
			if (p->Head->Type == ValueTypeNull) return "()";
			else if (p->Tail->Type == ValueTypeNull)
				ss << "(" << p->Head->String() << ")";
			else
				ss << "(" << p->Head->String() << " . " << p->Tail->String() << ")";
			
			return ss.str();
		}
		
		case ValueTypeBoolean:
			return (((BooleanValue*)this)->Value) ? "#t" : "#f";
		
		default: break;
	}
	return "??";
}
