#include "includes.h"
#include "Parser.h"
#include "Token.h"
#include <time.h>

//  TokenFactory <
TokenFactory::TokenFactory ()
{
	srand(time(NULL));
	/*std::stringstream ss;
	for (int i = 0; i < 6; i++)
		ss << (char)('a' + (rand() % 26));
	name = ss.str();*/
	
}
TokenFactory::~TokenFactory ()
{
	for (auto i = numbers.begin(); i != numbers.end(); ++i) delete *i;
	for (auto i = variables.begin(); i != variables.end(); ++i) delete *i;
	for (auto i = others.begin(); i != others.end(); ++i) delete *i;
	
}
NumberToken* TokenFactory::CreateNumberToken (Number n)
{
	for (auto i = numbers.begin(); i != numbers.end(); i++)
		if ((*i)->Value == n)
			return *i;
	NumberToken* t = new NumberToken(n);
	numbers.push_back(t);
	return t;
}
NumberToken* TokenFactory::CreateNumberToken (const std::string& s)
{
	return CreateNumberToken(atof(s.c_str()));
}
VariableToken* TokenFactory::CreateVariableToken (std::string s)
{ 
	for (auto i = variables.begin(); i != variables.end(); i++)
		if ((*i)->Name == s)
			return *i;
	VariableToken* t = new VariableToken(s);
	variables.push_back(t);
	return t;
}
ExpressionToken* TokenFactory::CreateExpressionToken (std::vector<Token*> tokens)
{
	Token* head = tokens.front();
	std::vector<Token*> tail = tokens;
	tail.erase(tail.begin());
	
	ExpressionToken* t = new ExpressionToken(head, tail);
	others.push_back(t);
	return t;
}
// >

//#define DEBUGGING_PARSER





#define BEGIN_EXP '('
#define END_EXP ')'



/*
static inline bool char_eq (char a, std::string b)
{
	std::string _a(a);
	return _a == b;
}
static inline bool char_eq (char a, char b) { return a == b; }
*/
#define char_eq(a,b) ((a)==(b))


#ifdef DEBUGGING_PARSER
#define pdebug(x) std::cerr << x << std::endl;
#else
#define pdebug(x) ;
#endif



static inline bool isblock (char q)
{
	return isspace(q) || q == ')';
}
static inline bool isnum (char c)
{
	return isdigit(c) || c == '.';
}




Parser::Parser ()
	: _factory(NULL), buffer("")
{
}
Parser::~Parser ()
{
	delete _factory;
}



TokenFactory* Parser::GetFactory ()
{
	if (_factory == NULL)
		_factory = new TokenFactory();
	return _factory;
}
TokenFactory* Parser::ObtainFactory ()
{
	TokenFactory* f = _factory;
	_factory = NULL;
	return f;
}
void Parser::Parse (std::string data)
{
	buffer = data;
	parse();
}
void Parser::Parse (const std::stringstream& data)
{
	buffer = data.str();
	parse();
}
void Parser::Parse (std::fstream& file)
{
	std::stringstream ss("");
	
	std::string line;
	
	while (!file.eof())
	{
		std::getline(file, line);
		ss << line << std::endl;
	}
	buffer = ss.str();
	
	parse();
}
char Parser::first ()
{
	if (eof()) return '\0';
	return buffer[0];
}
char Parser::next ()
{
	if (eof()) return '\0';
	char c = first();
	buffer = buffer.substr(1);
	return c;
}
bool Parser::eof ()
{
	return buffer.size() == 0;
}
void Parser::trim ()
{
	while (!eof() && isspace(first()))
		next();
}
std::string Parser::block ()
{
	std::stringstream out("");
	while (!eof() && !isblock(first()))
		out << next();
	return out.str();
}


Token* Parser::parseToken ()
{
	if (char_eq(first(), BEGIN_EXP))
	{
		next();
		pdebug("Reading expression, [" << buffer << "]");
		std::vector<Token*> tokens;
		
		trim();
		while (!eof() && !char_eq(first(), END_EXP))
		{
			tokens.push_back(parseToken());
			trim();
		}
		pdebug("Reached closing bracket");
		if (eof())
			die("Reached eof before ')'");
		
		next();
		
		return GetFactory()->CreateExpressionToken(tokens);
	}
	else
	{
		std::string input(block());
		
		if (input.size() == 0) {
			die("Empty variable or something");
		}
		
		bool isNumber = false;
		
		if (input[0] == '-')
			isNumber = (input.size() > 1);
		else
			isNumber = isnum(input[0]);
		
		
		pdebug("Read block [" << input << "] [" << buffer << "]");
		
		if (isNumber)
			return GetFactory()->CreateNumberToken(input);
		
		return GetFactory()->CreateVariableToken(input);
	}
	return NULL;
}




void Parser::parse ()
{
	trim();
	while (!eof())
	{
		parsed.push_back(parseToken());
		trim();
		pdebug("Parsed one token, {" << buffer << "}");
	}
	pdebug("finished parsing");
}

std::vector<Token*> Parser::FlushTokens ()
{
	std::vector<Token*> p = parsed;
	parsed.clear();
	return p;
}




/*============ Garbage collection ===============*/


void TokenFactory::GarbageCollectBegin ()
{
	gcLeft.clear();
	gcFound.clear();
	
	for (auto i = numbers.begin();   i != numbers.end();   i++) gcLeft.push_back(*i);
	for (auto i = variables.begin(); i != variables.end(); i++) gcLeft.push_back(*i);
	for (auto i = others.begin();    i != others.end();    i++) gcLeft.push_back(*i);
	
	numbers.clear(); variables.clear(); others.clear();
}
void TokenFactory::GarbageCollectProcess (Token* t)
{
	for (auto i = gcLeft.begin(); i != gcLeft.end(); i++)
	{
		if ((*i) == t)
		{
			gcLeft.erase(i);
			gcFound.push_back(t);
			return;
		}
	}
}
void TokenFactory::GarbageCollectEnd ()
{
	/*for (auto i = gcLeft.begin(); i != gcLeft.end(); i++)
	{
		std::cout << "destroying token "; displayToken(*i);
	}*/
	for (auto i = gcLeft.begin(); i != gcLeft.end(); i++)
		delete *i;
	gcLeft.clear();
	
	for (auto i = gcFound.cbegin(); i != gcFound.cend(); i++)
	{
		switch ((*i)->Type)
		{
			case TokenTypeNumber:   numbers.push_back((NumberToken*)*i); break;
			case TokenTypeVariable: variables.push_back((VariableToken*)*i); break;
			default: others.push_back(*i);
		}
		//std::cout << "keeping token "; displayToken(*i);
	}
	gcFound.clear();
}

bool TokenFactory::Empty ()
{
	return numbers.empty() && variables.empty() && others.empty();
}
