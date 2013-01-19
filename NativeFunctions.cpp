#include "includes.h"
#include "Interpreter.h"

#define _USE_MATH_DEFINES
#include <cmath>

static SValue* proc_add (std::vector<SValue*>& values)
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
static SValue* proc_sub (std::vector<SValue*>& values)
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
static SValue* proc_mult (std::vector<SValue*>& values)
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
static SValue* proc_div (std::vector<SValue*>& values)
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
static SValue* proc_cons (std::vector<SValue*>& values)
{
	if (values.size() != 2)
		die("Incorrect arguments to cons");
	
	return new PairValue(values[0]->Copy(), values[1]->Copy());
}
static SValue* proc_isnull (std::vector<SValue*>& values)
{
	for (auto i = values.begin(); i != values.end(); ++i)
		if ((*i)->Type != ValueTypeNull)
			return new BooleanValue(false);
	return new BooleanValue(true);
}
static SValue* proc_eql (std::vector<SValue*>& values)
{
	if (values.size() != 2)
	{
		die("Invalid number of arguments to =");
	}
	if (values[0]->Type != values[1]->Type)
	{
		return new BooleanValue(false);
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
static SValue* proc_less (std::vector<SValue*>& values)
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
static SValue* proc_grt (std::vector<SValue*>& values)
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
static SValue* proc_lsse (std::vector<SValue*>& values)
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
static SValue* proc_grte (std::vector<SValue*>& values)
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

static SValue* proc_pow (std::vector<SValue*>& values)
{
	if (values.size() != 2)
	{
		die("Invalid number of arguments to ^");
	}
	if (values[0]->Type != ValueTypeNumber || values[1]->Type != ValueTypeNumber)
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
static SValue* proc_rem (std::vector<SValue*>& values)
{
	if (values.size() != 2 || values[0]->Type != ValueTypeNumber || values[1]->Type != ValueTypeNumber)
	{
		die("Invalid arguments to %");
	}
	Number a = ((NumberValue*)values[0])->Value;
	Number b = ((NumberValue*)values[1])->Value;
	
	return new NumberValue(fmod(a, b));
}
static SValue* proc_sqrt (std::vector<SValue*>& values)
{
	if (values.size() != 1 || values[0]->Type != ValueTypeNumber)
	{
		die("Invalid arguments");
	}
	return new NumberValue(sqrt(((NumberValue*)values[0])->Value));
}
static SValue* proc_sin (std::vector<SValue*>& values)
{
	if (values.size() != 1 || values[0]->Type != ValueTypeNumber)
	{
		die("Invalid arguments");
	}
	return new NumberValue(sin(((NumberValue*)values[0])->Value));
}
static SValue* proc_cos (std::vector<SValue*>& values)
{
	if (values.size() != 1 || values[0]->Type != ValueTypeNumber)
	{
		die("Invalid arguments");
	}
	return new NumberValue(cos(((NumberValue*)values[0])->Value));
}
static SValue* proc_list (std::vector<SValue*>& values)
{
	if (values.size() == 0)
		return new NullValue();
	
	
	NullValue* nil = new NullValue();
	
	auto i = values.begin();
	
	PairValue* base = new PairValue((*i)->Copy(), nil);
	PairValue* pair = base;
	PairValue* cur;
	
	for (i++; i != values.end(); i++)
	{
		cur = new PairValue((*i)->Copy(), nil);
		pair->Tail = cur;
		pair = cur;
		//std::cout << "adding to list " << values[i]->String() << std::endl;
	}
	return base;
}
static SValue* proc_head (std::vector<SValue*>& values)
{
	if (values.size() != 1 || values[0]->Type != ValueTypePair)
	{
		die("Invalid arguments");
	}
	return ((PairValue*)values[0])->Head->Copy();
}
static SValue* proc_tail (std::vector<SValue*>& values)
{
	if (values.size() != 1 || values[0]->Type != ValueTypePair)
	{
		die("Invalid arguments");
	}
	return ((PairValue*)values[0])->Tail->Copy();
}



void RegisterNativeFunctions (Scope* s)
{
	s->Set("+", new NativeFunctionValue(proc_add));
	s->Set("-", new NativeFunctionValue(proc_sub));
	s->Set("*", new NativeFunctionValue(proc_mult));
	s->Set("/", new NativeFunctionValue(proc_div));
	s->Set("^", new NativeFunctionValue(proc_pow));
	s->Set("%", new NativeFunctionValue(proc_rem));
	
	s->Set("sqrt", new NativeFunctionValue(proc_sqrt));
	s->Set("sin", new NativeFunctionValue(proc_sin));
	s->Set("cos", new NativeFunctionValue(proc_cos));
	
	s->Set("PI", new NumberValue(3.14159265358979));
	
	s->Set("cons", new NativeFunctionValue(proc_cons));
	s->Set("list", new NativeFunctionValue(proc_list));
	s->Set("head", new NativeFunctionValue(proc_head));s->Set("car", new NativeFunctionValue(proc_head));
	s->Set("tail", new NativeFunctionValue(proc_tail));s->Set("cdr", new NativeFunctionValue(proc_tail));
	s->Set("null?", new NativeFunctionValue(proc_isnull));
	
	
	s->Set("=", new NativeFunctionValue(proc_eql));
	s->Set("<", new NativeFunctionValue(proc_less));
	s->Set("<=", new NativeFunctionValue(proc_lsse));
	s->Set(">", new NativeFunctionValue(proc_grt));
	s->Set(">=", new NativeFunctionValue(proc_grte));
}
