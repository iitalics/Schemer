#pragma once


typedef long double Number;


class Token;class ExpressionToken;class Interpreter;class Scope;


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
	FunctionTypeLambda = 2 + 4 
};


class SValue
{
public:
	ValueType Type;
	virtual ~SValue ();
	
	virtual SValue* Copy () = 0;
	
	std::string String ();
	
	
	static SValue* Create (Number);
	static SValue* Create (bool);
	static SValue* Create (SValue*, SValue*);
};

class NullValue : public SValue { public: virtual SValue* Copy(); };

class BooleanValue : public SValue
{
public:
	BooleanValue (bool b);
	virtual SValue* Copy ();
	bool Value;
};


class NumberValue : public SValue
{
public:
	NumberValue (Number n);
	Number Value;
	
	virtual SValue* Copy ();
};
class PairValue : public SValue
{
public:
	PairValue (SValue* head, SValue* tail);
	virtual ~PairValue ();
	SValue* Head;
	SValue* Tail;
	
	virtual SValue* Copy ();
};
class FunctionValue : public SValue
{
public:
	FunctionType fType;
	
	virtual SValue* Call (Interpreter*, std::vector<SValue*>&) = 0;
	virtual std::string Name () = 0;
};
class NormalFunctionValue : public FunctionValue
{
public:
	NormalFunctionValue (ExpressionToken* args, Token* body);
	virtual SValue* Call (Interpreter* n, std::vector<SValue*>& args);
	
	virtual SValue* Copy ();
	virtual std::string Name ();
	
	ExpressionToken* Arguments;
	Token* Body;
	
protected:
	NormalFunctionValue ();
};

class LambdaFunctionValue : public NormalFunctionValue
{
public:
	LambdaFunctionValue (ExpressionToken* args, Token* body, Scope* scope);
	virtual ~LambdaFunctionValue ();
	virtual SValue* Call (Interpreter* n, std::vector<SValue*>& args);
	
	virtual SValue* Copy ();
	virtual std::string Name ();
	
	Scope* scope;
	
};


typedef SValue* (*NativeFunctionHandler)(std::vector<SValue*>&);

class NativeFunctionValue : public FunctionValue
{
public:
	NativeFunctionValue (NativeFunctionHandler handler);
	virtual SValue* Call (Interpreter* n, std::vector<SValue*>& args);
	virtual SValue* Copy ();
	
	virtual std::string Name ();
	
	NativeFunctionHandler Handler;
};
