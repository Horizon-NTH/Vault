#include "Application.h"
#include <iostream>
#include "Utils.h"

int main(const int argc, const char* argv[])
{
	Application app(std::span(argv, argc));
	return app.execute();
}
