#include "includes.h"
#include "Value.h"
#include "Parser.h"
#include "Token.h"
#include "Interpreter.h"

extern "C"
{
	#include "argvments.h"
}


#define VERSION "1.0"




#define BANNER "\
\n\
 :::::::::::\n\
   Schemer\n\
 :::::::::::\n\n"



/*
 * 
 * Give files to read as arguments, or give no arguments for RPL
 * 
 */



static bool doing_rpl = true;
static std::stringstream fileInput;



static void rpl ()
{
	Parser parser;
	Interpreter interp;
	
	std::string line;
	std::string output;
	
	std::cout << BANNER;
	
	for (;;)
	{
		std::cout << "> ";
		if (!std::getline(std::cin, line))
			break;
		
		parser.Parse(line);
		interp.LoadParser(parser);
		interp.Run();
		interp.GarbageCollect();
		
		while (interp >> output)
			std::cout << output << std::endl;
	}
	
	std::cout << std::endl;
}

static void runCode ()
{
	Parser parser;
	Interpreter interp;
	
	parser.Parse(fileInput);
	interp.LoadParser(parser);
	interp.Run();
	
	std::string output;
	
	while (interp >> output)
		std::cout << output << std::endl;
}



static void option_basic (char* _input, int N)
{
	doing_rpl = false;
	
	std::string line;
	std::fstream file(_input);
	
	if (file.good())
	{
		while (!file.eof())
		{
			std::getline(file, line);
			fileInput << line << std::endl;
		}
		
		file.close();
	}
	else
	{
		die("Unable to open file '" << _input << "'");
	}
}
static void option_c (char* _input, int N)
{
	doing_rpl = false;
	
	fileInput << _input << std::endl;
}



int main (int argc, char** argv)
{
	fileInput.str("");
	
	argvm_begin(argc, argv);
	
	argvm_no_arg_help(false);
	argvm_usage_text("[FILE1, FILE2, ...] [--code]\n Executes files in arguments, or goes into RPL mode if no arguments are given\n");
	argvm_version_text("Schemer version " VERSION);
	
	argvm_option(0, "code", true, option_c, "Execute command line input");
	argvm_basic(option_basic, NULL);
	
	if (argvm_end() == ARGVM_SUCCESS)
	{
		if (doing_rpl)
			rpl();
		else
			runCode();
	}
	
#ifdef WATCH_MEMORY
	showUsage();
#endif
	
	return EXIT_SUCCESS;
}
