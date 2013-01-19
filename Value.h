#pragma once


typedef double Number;


class Token;class ExpressionToken;class Interpreter;

enum ValueType
{
	ValueTypeNumber = 1,
	ValueTypeFunction = 2,
	ValueTypePair = 3,
	ValueTypeBoolean = 4,
	ValueTypeNull = 0
};
enum FunctionType
{
	FunctionTypeNative = 1,
	FunctionTypeNormal = 2,
	FunctionTypeLambda = 3
};


struct SValue
{
	ValueType Type;
	virtual ~SValue ();
	
	virtual SValue* Copy () = 0;
	
	std::string String ();
	
	
	static SValue* Create (Number);
	static SValue* Create (bool);
	static SValue* Create (SValue*, SValue*);
};

struct NullValue : public SValue { virtual SValue* Copy(); };

struct BooleanValue : public SValue
{
	BooleanValue (bool b);
	virtual SValue* Copy ();
	bool Value;
};


struct NumberValue : public SValue
{
	NumberValue (Number n);
	Number Value;
	
	virtual SValue* Copy ();
};
struct PairValue : public SValue
{
	PairValue (SValue* head, SValue* tail);
	virtual ~PairValue ();
	SValue* Head;
	SValue* Tail;
	
	virtual SValue* Copy ();
};
struct FunctionValue : public SValue
{
	FunctionType fType;
	
	virtual SValue* Call (Interpreter*, std::vector<SValue*>&) = 0;
	virtual std::string Name () = 0;
};
struct NormalFunctionValue : public FunctionValue
{
	NormalFunctionValue (ExpressionToken* args, Token* body); // scope?
	virtual SValue* Call (Interpreter* n, std::vector<SValue*>& args);
	
	virtual SValue* Copy ();
	virtual std::string Name ();
	
	ExpressionToken* Arguments;
	Token* Body;
};

typedef SValue* (*NativeFunctionHandler)(std::vector<SValue*>&);

struct NativeFunctionValue : public FunctionValue
{
	NativeFunctionValue (NativeFunctionHandler handler);
	virtual SValue* Call (Interpreter* n, std::vector<SValue*>& args);
	virtual SValue* Copy ();
	
	virtual std::string Name ();
	
	NativeFunctionHandler Handler;
};
