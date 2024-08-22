#include "Application.h"

int main(const int argc, const char* argv[])
{
	const Application app(std::span(argv, argc));
	app.execute();
	return 0;
}
