#pragma once

#include <cstddef>


void* operator new(std::size_t sz);
void  operator delete(void* p);
