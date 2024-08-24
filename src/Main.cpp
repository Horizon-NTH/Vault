#include "Application.h"

#include <iostream>

int main(const int argc, const char* argv[])
{
	try
	{
		const Application app(std::span(argv, argc));
		app.execute();
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error: " << e.what() << '\n';
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
