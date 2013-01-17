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
SValue* NormalFunctionValue::Call (Interpreter* t, std::vector<SValue*> args)
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
SValue* NativeFunctionValue::Call (Interpreter* t, std::vector<SValue*> args)
{
	if (Handler == NULL) return new NullValue();
	return Handler(args);
}

std::string NormalFunctionValue::Name ()
{
	std::stringstream ss; 
	ss << "{function " << ((VariableToken*)Arguments->Function)->Name << "}";
	return ss.str();
}
std::string NativeFunctionValue::Name ()
{
	return "{native function}";
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





std::string SValue::String ()
{
	std::stringstream ss;
	switch (Type)
	{
		case ValueTypeNull:
			return "nil";
		
		case ValueTypeNumber:
			ss << ((NumberValue*)this)->Value;
			return ss.str();
		
		case ValueTypeFunction:
			return ((FunctionValue*)this)->Name();
		
		case ValueTypePair:
		{
			PairValue* p = (PairValue*)this;
			
			ss << "(" << p->Head->String() << " . " << p->Tail->String() << ")";
			return ss.str();
		}
		
		case ValueTypeBoolean:
			return (((BooleanValue*)this)->Value) ? "true" : "false";
		
		default: break;
	}
	return "??";
}
