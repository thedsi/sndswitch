#include "stdafx.h"
#include "memnew.h"


void* operator new(std::size_t sz)
{
	return LocalAlloc(0, sz);
}


void operator delete(void* p)
{
	LocalFree(p);
}
