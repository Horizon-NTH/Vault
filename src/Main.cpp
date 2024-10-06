#include "Application.h"
#include <iostream>

int main(const int argc, const char* argv[])
{
	Application app(std::span(argv, argc));
	return app.execute();
}
