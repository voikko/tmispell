#include <stdarg.h>

#include "tmerror.hh"

Error::Error(char* const fmt, ...)
	: std::runtime_error("")
{
	char buf[4096];
	va_list va;
	va_start(va, fmt);
	vsnprintf(buf, 4096, fmt, va);
	va_end(va);
	throw Error(std::string(buf));
}

Error::Error(std::string const& msg)
	: std::runtime_error(msg)
{
}
