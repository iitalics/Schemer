#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <map>




#ifdef WIN32

#define COLOR_Red ""
#define COLOR_Off ""

#else

#define COLOR_Red "\x1b[31;1m"
#define COLOR_Off "\x1b[0m"

#endif

#define die(x) {std::cerr << COLOR_Red << x << COLOR_Off << std::endl;exit(-1);}
