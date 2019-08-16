#pragma once
#ifndef SUPPORT_H
#define SUPPORT_H


// Allow GNU compiler to understand strcpy_s() of MSVC
#if !defined(_MSC_VER)
void strcpy_s(char dst[], const char* src);
#endif

#endif
