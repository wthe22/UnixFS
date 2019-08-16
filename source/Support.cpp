#include <cstring>
#include "Support.h"
using namespace std;


// Allow GNU compiler to understand strcpy_s() of MSVC
#if !defined(_MSC_VER)
void strcpy_s(char dst[], const char* src) {
	strcpy(dst, src);
}
#endif
