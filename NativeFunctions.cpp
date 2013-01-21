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
		die("Invalid arguments to sqrt");
	}
	return new NumberValue(sqrt(((NumberValue*)values[0])->Value));
}
static SValue* proc_sin (std::vector<SValue*>& values)
{
	if (values.size() != 1 || values[0]->Type != ValueTypeNumber)
	{
		die("Invalid arguments to sin");
	}
	return new NumberValue(sin(((NumberValue*)values[0])->Value));
}
static SValue* proc_cos (std::vector<SValue*>& values)
{
	if (values.size() != 1 || values[0]->Type != ValueTypeNumber)
	{
		die("Invalid arguments to co");
	}
	return new NumberValue(cos(((NumberValue*)values[0])->Value));
}
static SValue* proc_floor (std::vector<SValue*>& values)
{
	if (values.size() != 1 || values[0]->Type != ValueTypeNumber)
	{
		die("Invalid arguments to floor");
	}
	return new NumberValue((int)(((NumberValue*)values[0])->Value));
}
static SValue* proc_ceil (std::vector<SValue*>& values)
{
	if (values.size() != 1 || values[0]->Type != ValueTypeNumber)
	{
		die("Invalid arguments to ceil");
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
		die("Invalid arguments to head/car");
	}
	return ((PairValue*)values[0])->Head->Copy();
}
static SValue* proc_tail (std::vector<SValue*>& values)
{
	if (values.size() != 1 || values[0]->Type != ValueTypePair)
	{
		die("Invalid arguments to tail/cdr");
	}
	return ((PairValue*)values[0])->Tail->Copy();
}
static SValue* proc_idx (std::vector<SValue*>& values)
{
	if (values.size() != 2 ||
		values[0]->Type != ValueTypePair ||
		values[1]->Type != ValueTypeNumber)
	{
		die("Invalid arguments to idx");
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
static SValue* proc_length (std::vector<SValue*>& values)
{
	if (values.size() != 1 ||
			values[0]->Type != ValueTypePair)
		die("Invalid arguments to length");
	int length;
	PairValue* pair = ((PairValue*)values[0]);
	
	for (length = 1; pair->Tail->Type == ValueTypePair; length++)
		pair = (PairValue*)pair->Tail;
	return new NumberValue(length);
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
	
	if (line.size() == 0)
		return new NullValue();
	
	return new NumberValue(atof(line.c_str()));
}
static SValue* proc_input_str (std::vector<SValue*>& values)
{
	std::string line;
	getline(std::cin, line);
	
	if (line.size() == 0)
		return new NullValue();
	
	return new StringValue(line);
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
static SValue* proc_string_idx (std::vector<SValue*>& values)
{
	if (values.size() != 2 ||
		values[0]->Type != ValueTypeString ||
		values[1]->Type != ValueTypeNumber)
	{
		die("Invalid arguments");
	}
	unsigned int i = (unsigned int)((NumberValue*)values[1])->Value;
	std::string s(((StringValue*)values[0])->Text);
	
	if (i < 0 || i >= s.size())
		return new NullValue();
	return new NumberValue(s[i]);
}
static SValue* proc_string_len (std::vector<SValue*>& values)
{
	if (values.size() != 1 ||
		values[0]->Type != ValueTypeString)
		die("Invalid arguments to string-length");
	std::string s(((StringValue*)values[0])->Text);
	return new NumberValue(s.size());
}
static SValue* proc_string (std::vector<SValue*>& values)
{
	std::stringstream ss;
	for (auto i = values.cbegin(); i != values.cend(); i++)
	{
		if ((*i)->Type == ValueTypeString)
			ss << (((StringValue*)*i)->Text);
		else
			ss << (*i)->String();
	}
	return new StringValue(ss.str());
}
static SValue* proc_string_char (std::vector<SValue*>& values)
{
	std::stringstream ss;
	for (auto i = values.cbegin(); i != values.cend(); i++)
	{
		if ((*i)->Type != ValueTypeNumber)
			die("Invalid non-number argument");
		
		ss << (char)(((NumberValue*)*i)->Value);
	}
	return new StringValue(ss.str());
}
static SValue* proc_isstring (std::vector<SValue*>& values)
{
	if (values.size() != 1) die("Invalid arguments");
	return new BooleanValue(values[0]->Type == ValueTypeString);
}
static SValue* proc_ispair (std::vector<SValue*>& values)
{
	if (values.size() != 1) die("Invalid arguments");
	return new BooleanValue(values[0]->Type == ValueTypePair);
}
static SValue* proc_isnumber (std::vector<SValue*>& values)
{
	if (values.size() != 1) die("Invalid arguments");
	return new BooleanValue(values[0]->Type == ValueTypeNumber);
}
static SValue* proc_isbool (std::vector<SValue*>& values)
{
	if (values.size() != 1) die("Invalid arguments");
	return new BooleanValue(values[0]->Type == ValueTypeBoolean);
}
static SValue* proc_rand (std::vector<SValue*>& values)
{
	Number n = rand() / (Number)RAND_MAX;
	if (values.size() == 1 && values[0]->Type == ValueTypeNumber)
		n *= ((NumberValue*)values[0])->Value;
	return new NumberValue(n);
}






static std::vector<SValue*> values;

template <class Q>
static Q _PushValue (Q v)
{
	values.push_back(v);
	return v;
}
#define PushValue(x) _PushValue<decltype(x)>(x)
static NativeFunctionValue* MakeFunction (NativeFunctionHandler handler)
{
	return PushValue(new/**/NativeFunctionValue(handler));
}

void RegisterNativeFunctions (Scope* s)
{
	s->Set("+", MakeFunction(proc_add));
	s->Set("-", MakeFunction(proc_sub));
	s->Set("*", MakeFunction(proc_mult));
	s->Set("/", MakeFunction(proc_div));
	s->Set("^", MakeFunction(proc_pow));
	s->Set("%", MakeFunction(proc_rem));
	
	s->Set("sqrt", MakeFunction(proc_sqrt));
	s->Set("sin", MakeFunction(proc_sin));
	s->Set("cos", MakeFunction(proc_cos));
	s->Set("floor", MakeFunction(proc_floor));
	s->Set("ceil", MakeFunction(proc_ceil));
	
	s->Set("PI", PushValue(new NumberValue(3.14159265358979)));
	
	s->Set("cons", MakeFunction(proc_cons));
	s->Set("list", MakeFunction(proc_list));
	s->Set("head", MakeFunction(proc_head));s->Set("car", MakeFunction(proc_head));
	s->Set("tail", MakeFunction(proc_tail));s->Set("cdr", MakeFunction(proc_tail));
	s->Set("index", MakeFunction(proc_idx));
	s->Set("length", MakeFunction(proc_length));
	
	s->Set("null?", MakeFunction(proc_isnull));
	s->Set("string?", MakeFunction(proc_isstring));
	s->Set("pair?", MakeFunction(proc_ispair));
	s->Set("number?", MakeFunction(proc_isnumber));
	s->Set("bool?", MakeFunction(proc_isbool));
	
	s->Set("string-at", MakeFunction(proc_string_idx));
	s->Set("string", MakeFunction(proc_string));
	s->Set("string-char", MakeFunction(proc_string_char));
	s->Set("string-length", MakeFunction(proc_string_len));
	
	s->Set("=", MakeFunction(proc_eql));
	s->Set("!=", MakeFunction(proc_neql));
	s->Set("<", MakeFunction(proc_less));
	s->Set("<=", MakeFunction(proc_lsse));
	s->Set(">", MakeFunction(proc_grt));
	s->Set(">=", MakeFunction(proc_grte));
	
	s->Set("and", MakeFunction(proc_and));
	s->Set("or", MakeFunction(proc_or));
	s->Set("not", MakeFunction(proc_not));
	
	
	s->Set("display", MakeFunction(proc_display));
	s->Set("new-line", MakeFunction(proc_newline));
	s->Set("input", MakeFunction(proc_input));
	s->Set("input-string", MakeFunction(proc_input_str));
	s->Set("sleep", MakeFunction(proc_sleep));
	s->Set("exit", MakeFunction(proc_exit));
	s->Set("rand", MakeFunction(proc_rand));
	
	for (auto i = values.cbegin(); i != values.cend(); i++)
		delete *i;
	values.clear();
}
