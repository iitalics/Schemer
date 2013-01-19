#include "includes.h"
#include "Interpreter.h"

#define _USE_MATH_DEFINES
#include <cmath>

static SValue* proc_add (std::vector<SValue*> values)
{
	Number r = 0;
	for (auto i = values.begin(); i != values.end(); ++i)
	{
		if ((*i)->Type != ValueTypeNumber)
			die((*i)->String() << " is not a number")//;
		r += ((NumberValue*)*i)->Value;
	}
	return new NumberValue(r);
}
static SValue* proc_sub (std::vector<SValue*> values)
{
	if (values.size() == 0) return 0;
	auto i = values.begin();
	
	if ((*i)->Type != ValueTypeNumber) die((*i)->String() << " is not a number");
	Number r = ((NumberValue*)*i)->Value;
	
	for (i++; i != values.end(); i++)
	{
		if ((*i)->Type != ValueTypeNumber) die((*i)->String() << " is not a number");
		r -= ((NumberValue*)*i)->Value;
	}
	return new NumberValue(r);
}
static SValue* proc_mult (std::vector<SValue*> values)
{
	Number r = 1;
	for (auto i = values.begin(); i != values.end(); ++i)
	{
		if ((*i)->Type != ValueTypeNumber)
			die((*i)->String() << " is not a number")//;
		r *= ((NumberValue*)*i)->Value;
	}
	return new NumberValue(r);
}
static SValue* proc_div (std::vector<SValue*> values)
{
	if (values.size() == 0) return 0;
	auto i = values.begin();
	
	if ((*i)->Type != ValueTypeNumber) die((*i)->String() << " is not a number");
	Number r = ((NumberValue*)*i)->Value;
	
	for (i++; i != values.end(); i++)
	{
		if ((*i)->Type != ValueTypeNumber) die((*i)->String() << " is not a number");
		r /= ((NumberValue*)*i)->Value;
	}
	return new NumberValue(r);
}
/*
static SValue* proc_cond (std::vector<SValue*> values)
{
	if (values.size() != 2)
		die("Incorrect arguments to (cond)");
	
	return new PairValue(values[0], values[1]);
}*/
static SValue* proc_isnull (std::vector<SValue*> values)
{
	for (auto i = values.begin(); i != values.end(); ++i)
		if ((*i)->Type != ValueTypeNull)
			return new BooleanValue(false);
	return new BooleanValue(true);
}
static SValue* proc_eql (std::vector<SValue*> values)
{
	if (values.size() != 2)
	{
		die("Invalid number of arguments to =");
	}
	if (values[0]->Type != values[1]->Type)
	{
		return new BooleanValue(false);
		//die("Invalid conflicting types");
	}
	
	bool v = false;
	switch (values[0]->Type)
	{
		case ValueTypeNumber:
			v = ((NumberValue*)values[0])->Value == ((NumberValue*)values[1])->Value;
			break;
		case ValueTypeBoolean:
			v = ((BooleanValue*)values[0])->Value == ((BooleanValue*)values[1])->Value;
			break;
		case ValueTypePair:
		case ValueTypeFunction:
			v = values[0] == values[1];
			break;
		
		default: break;
	}
	return new BooleanValue(v);
}
static SValue* proc_less (std::vector<SValue*> values)
{
	if (values.size() != 2)
	{
		die("Invalid number of arguments to <");
	}
	if (values[0]->Type != ValueTypeNumber)
	{
		die("Invalid comparing non-numbers");
	}
	return new BooleanValue(
		((NumberValue*)values[0])->Value
			<
		((NumberValue*)values[1])->Value);
}
static SValue* proc_grt (std::vector<SValue*> values)
{
	if (values.size() != 2)
	{
		die("Invalid number of arguments to >");
	}
	if (values[0]->Type != ValueTypeNumber)
	{
		die("Invalid comparing non-numbers");
	}
	return new BooleanValue(
		((NumberValue*)values[0])->Value
			>
		((NumberValue*)values[1])->Value);
}
static SValue* proc_lsse (std::vector<SValue*> values)
{
	if (values.size() != 2)
	{
		die("Invalid number of arguments to <=");
	}
	if (values[0]->Type != ValueTypeNumber)
	{
		die("Invalid comparing non-numbers");
	}
	return new BooleanValue(
		((NumberValue*)values[0])->Value
			<=
		((NumberValue*)values[1])->Value);
}
static SValue* proc_grte (std::vector<SValue*> values)
{
	if (values.size() != 2)
	{
		die("Invalid number of arguments to >=");
	}
	if (values[0]->Type != ValueTypeNumber)
	{
		die("Invalid comparing non-numbers");
	}
	return new BooleanValue(
		((NumberValue*)values[0])->Value
			>=
		((NumberValue*)values[1])->Value);
}

static SValue* proc_pow (std::vector<SValue*> values)
{
	if (values.size() != 2)
	{
		die("Invalid number of arguments to ^");
	}
	if (values[0]->Type != ValueTypeNumber)
	{
		die("Invalid exponents on non-numbers");
	}
	Number a = ((NumberValue*)values[0])->Value;
	Number b = ((NumberValue*)values[1])->Value;
	
	if (a == 1 || b == 0)
		return new NumberValue(1);
	if (b == 0.5)
		return new NumberValue(sqrt(a));
	if (b == 2)
		return new NumberValue(a * a);
	return new NumberValue(pow(a, b));
}
static SValue* proc_sqrt (std::vector<SValue*> values)
{
	if (values.size() != 1 || values[0]->Type != ValueTypeNumber)
	{
		die("Invalid arguments");
	}
	return new NumberValue(sqrt(((NumberValue*)values[0])->Value));
}
static SValue* proc_sin (std::vector<SValue*> values)
{
	if (values.size() != 1 || values[0]->Type != ValueTypeNumber)
	{
		die("Invalid arguments");
	}
	return new NumberValue(sin(((NumberValue*)values[0])->Value));
}
static SValue* proc_cos (std::vector<SValue*> values)
{
	if (values.size() != 1 || values[0]->Type != ValueTypeNumber)
	{
		die("Invalid arguments");
	}
	return new NumberValue(cos(((NumberValue*)values[0])->Value));
}



void RegisterNativeFunctions (Scope* s)
{
	s->Set("+", new NativeFunctionValue(proc_add));
	s->Set("-", new NativeFunctionValue(proc_sub));
	s->Set("*", new NativeFunctionValue(proc_mult));
	s->Set("/", new NativeFunctionValue(proc_div));
	s->Set("^", new NativeFunctionValue(proc_pow));
	
	s->Set("sqrt", new NativeFunctionValue(proc_sqrt));
	s->Set("sin", new NativeFunctionValue(proc_sin));
	s->Set("cos", new NativeFunctionValue(proc_cos));
	
	s->Set("PI", new NumberValue(3.14159265358979));
	
	//s->Set("cond", new NativeFunctionValue(proc_cond));
	s->Set("null?", new NativeFunctionValue(proc_isnull));
	
	
	s->Set("=", new NativeFunctionValue(proc_eql));
	s->Set("<", new NativeFunctionValue(proc_less));
	s->Set("<=", new NativeFunctionValue(proc_lsse));
	s->Set(">", new NativeFunctionValue(proc_grt));
	s->Set(">=", new NativeFunctionValue(proc_grte));
}
