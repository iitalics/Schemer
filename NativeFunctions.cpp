#include <cmath>
#include <cstdlib>
#include <unistd.h>
#include "includes.h"
#include "Interpreter.h"

static SValue* proc_add (std::vector<SValue*>& values)
{
	Number r = 0;
	for (auto i = values.begin(); i != values.end(); ++i)
	{
		if ((*i)->Type != ValueTypeNumber)
			die((*i)->String() << " is not a number");
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
			die((*i)->String() << " is not a number");
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
	if (values.size() == 0 || values.size() > 2)
		die("Incorrect arguments to cons");
	
	return new PairValue(values[0]->Copy(),
		values.size() == 2 ? values[1]->Copy() : new NullValue());
}
static SValue* proc_isnull (std::vector<SValue*>& values)
{
	for (auto i = values.begin(); i != values.end(); ++i)
		if ((*i)->Type != ValueTypeNull)
			return new BooleanValue(false);
	return new BooleanValue(true);
}

static bool values_eql (SValue* a, SValue* b);
static bool values_eql (SValue* a, SValue* b)
{
	if (a->Type != b->Type) return false;
	
	switch (a->Type)
	{
		case ValueTypeNumber:
			return (((NumberValue*)a)->Value) == (((NumberValue*)b)->Value);
		case ValueTypeBoolean:
			return (((BooleanValue*)a)->Value) == (((BooleanValue*)b)->Value);
		case ValueTypeNull: return true;
		case ValueTypeFunction:
		{
			FunctionValue* fa = (FunctionValue*)a;
			FunctionValue* fb = (FunctionValue*)b;
			
			if (fa->fType != fb->fType) return false;
			
			if (fa->fType == FunctionTypeLambda) return false;
			if (fa->fType == FunctionTypeNative)
				return (((NativeFunctionValue*)fa)->Handler) == (((NativeFunctionValue*)fb)->Handler);
			if (fa->fType == FunctionTypeNormal)
			{
				NormalFunctionValue* na = (NormalFunctionValue*)a;
				NormalFunctionValue* nb = (NormalFunctionValue*)b;
				return na->Arguments == nb->Arguments &&
				       na->Body == nb->Body;
			}
			return false;
		}
		case ValueTypePair:
		{
			PairValue* pa = (PairValue*)a;
			PairValue* pb = (PairValue*)b;
			return values_eql(pa->Head, pb->Head) &&
			       values_eql(pa->Tail, pb->Tail);
		}
		default:
			return false;
	}
}

static SValue* proc_eql (std::vector<SValue*>& values)
{
	if (values.size() != 2)
	{
		die("Invalid number of arguments to =");
	}
	return new BooleanValue(values_eql(values[0], values[1]));
}
static SValue* proc_neql (std::vector<SValue*>& values)
{
	if (values.size() != 2)
	{
		die("Invalid number of arguments to =");
	}
	return new BooleanValue(!values_eql(values[0], values[1]));
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
static SValue* proc_floor (std::vector<SValue*>& values)
{
	if (values.size() != 1 || values[0]->Type != ValueTypeNumber)
	{
		die("Invalid arguments");
	}
	return new NumberValue((int)(((NumberValue*)values[0])->Value));
}
static SValue* proc_ceil (std::vector<SValue*>& values)
{
	if (values.size() != 1 || values[0]->Type != ValueTypeNumber)
	{
		die("Invalid arguments");
	}
	Number value = (((NumberValue*)values[0])->Value);
	
	return new NumberValue(floor(value));
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
static SValue* proc_idx (std::vector<SValue*>& values)
{
	if (values.size() != 2 ||
		values[0]->Type != ValueTypePair ||
		values[1]->Type != ValueTypeNumber)
	{
		die("Invalid arguments");
	}
	PairValue* pair = ((PairValue*)values[0]);
	int i = (int)((NumberValue*)values[1])->Value;
	
	while (i > 0)
	{
		if (pair->Tail->Type != ValueTypePair)
		{
			die("Index greater than list length");
		}
		pair = (PairValue*)pair->Tail;
		
		i--;
	}
	return pair->Head->Copy();
}
static SValue* proc_display (std::vector<SValue*>& values)
{
	for (auto i = values.begin(); i != values.end(); i++)
	{
		if ((*i)->Type == ValueTypeString)
			std::cout << ((StringValue*)*i)->Text;
		else
			std::cout << (*i)->String();
	}
	return new NullValue();
}
static SValue* proc_newline (std::vector<SValue*>& values)
{
	std::cout << std::endl;
	return new NullValue();
}
static SValue* proc_input (std::vector<SValue*>& values)
{
	std::string line;
	getline(std::cin, line);
	
	if (line.size() == 0 || line == "nil")
		return new NullValue();
	if (line == "#t")
		return new BooleanValue(true);
	if (line == "#f")
		return new BooleanValue(false);
	
	return new NumberValue(atof(line.c_str()));
}
static SValue* proc_exit (std::vector<SValue*>& values)
{
	if (values.size() == 0)
		std::exit(0);
	else if (values[0]->Type == ValueTypeNumber)
		std::exit((int)(((NumberValue*)values[0])->Value));
	else
		die("Invalid arguments to exit");
	
	return new NullValue();
}
static SValue* proc_sleep (std::vector<SValue*>& values)
{
	if (values.size() == 1 && values[0]->Type == ValueTypeNumber)
	{
		usleep((int)(
			((NumberValue*)values[0])->Value
			* 1000 * 1000));
	}
	else
		die("Invalid arguments to sleep");
	
	return new NullValue();
}
static SValue* proc_not (std::vector<SValue*>& values)
{
	if (values.size() != 1 || values[0]->Type != ValueTypeBoolean)
		die(values[0]->String() << " is not a boolean value");
	return new BooleanValue(!(((BooleanValue*)values[0])->Value));
}
static SValue* proc_and (std::vector<SValue*>& values)
{
	for (auto i = values.begin(); i != values.end(); i++)
	{
		BooleanValue* v = (BooleanValue*)*i;
		if (v->Type != ValueTypeBoolean)
			die("Invalid non-boolean argument");
		if (!v->Value)
			return new BooleanValue(false);
	}
	return new BooleanValue(true);
}
static SValue* proc_or (std::vector<SValue*>& values)
{
	for (auto i = values.begin(); i != values.end(); i++)
	{
		BooleanValue* v = (BooleanValue*)*i;
		if (v->Type != ValueTypeBoolean)
			die("Invalid non-boolean argument");
		if (v->Value)
			return new BooleanValue(true);
	}
	return new BooleanValue(false);
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
	s->Set("floor", new NativeFunctionValue(proc_floor));
	s->Set("ceil", new NativeFunctionValue(proc_ceil));
	
	s->Set("PI", new NumberValue(3.14159265358979));
	
	s->Set("cons", new NativeFunctionValue(proc_cons));
	s->Set("list", new NativeFunctionValue(proc_list));
	s->Set("head", new NativeFunctionValue(proc_head));s->Set("car", new NativeFunctionValue(proc_head));
	s->Set("tail", new NativeFunctionValue(proc_tail));s->Set("cdr", new NativeFunctionValue(proc_tail));
	s->Set("index", new NativeFunctionValue(proc_idx));
	s->Set("null?", new NativeFunctionValue(proc_isnull));
	
	
	s->Set("=", new NativeFunctionValue(proc_eql));
	s->Set("!=", new NativeFunctionValue(proc_neql));
	s->Set("<", new NativeFunctionValue(proc_less));
	s->Set("<=", new NativeFunctionValue(proc_lsse));
	s->Set(">", new NativeFunctionValue(proc_grt));
	s->Set(">=", new NativeFunctionValue(proc_grte));
	
	s->Set("and", new NativeFunctionValue(proc_and));
	s->Set("or", new NativeFunctionValue(proc_or));
	s->Set("not", new NativeFunctionValue(proc_not));
	
	
	s->Set("display", new NativeFunctionValue(proc_display));
	s->Set("new-line", new NativeFunctionValue(proc_newline));
	s->Set("input", new NativeFunctionValue(proc_input));
	s->Set("sleep", new NativeFunctionValue(proc_sleep));
	s->Set("exit", new NativeFunctionValue(proc_exit));
}
