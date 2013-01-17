#include "includes.h"
#include "Value.h"
#include "Parser.h"
#include "Token.h"
#include "Interpreter.h"



#define VERSION "1.0"



/*
 * 
 * Give file to read as first argument, or give no arguments for RPL
 * 
 */




int main (int argc, char** argv)
{
	
	std::string line;
	
	Parser parser;
	Interpreter interp;
	
	if (argc > 1)
	{
		std::fstream file(argv[1]);
		if (file.good())
		{
			parser.Parse(file);
			interp.LoadParser(parser);
			interp.Run();
			
			while (interp >> line)
				std::cout << line << std::endl;
			
			return EXIT_SUCCESS;
		}
		else
		{
			std::cerr << "\x1b[31mUnable to open file '" << argv[1] << "'" << std::endl;
			return EXIT_FAILURE;
		}
	}
	
	
	std::cout << std::endl
	          << " ::::::::::::::::::::::::::" << std::endl
	          << "  Schemer version " VERSION << std::endl
	          << " ::::::::::::::::::::::::::" << std::endl << std::endl;
	
	
	for (;;)
	{
		std::cout << "> ";
		std::getline(std::cin, line);
		
		if (std::cin.eof())
			break;
		
		parser.Parse(line);
		interp.LoadParser(parser);
		interp.Run();
		
		while ((interp >> line))
			std::cout << line << std::endl;
	}
	std::cout << std::endl;
	
	return EXIT_SUCCESS;
}
