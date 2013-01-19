#ifndef ARGVMENTS_H_INCLUDED
#define ARGVMENTS_H_INCLUDED

#include <stdbool.h>




typedef void (*argvm_Handler)(char*, int);

typedef enum
{
	ARGVM_SUCCESS   = 0,
	ARGVM_HELP_TEXT = 1
} argvm_Result;

extern void argvm_begin (int argc, char** argv);

extern void argvm_basic (argvm_Handler handler, char* help);
extern void argvm_option (char shortName, char* longNameOrNull, bool takesInput, argvm_Handler handler, char* help);
extern void argvm_no_arg_help (bool onoff);

extern void argvm_error_handler (void (*handler)(char*));
extern void argvm_usage_text (char* text);
extern void argvm_version_text (char* text);

extern argvm_Result argvm_end ();






#endif//ARGVMENTS_H_INCLUDED
