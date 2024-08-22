#pragma once

#include <functional>
#include <span>
#include "ArgumentsParser.h"

class Application
{
public:
	explicit Application(const std::span<const char*>& args);

	void execute() const;

private:
	ArgumentsParser m_parser;
	mutable std::function<void()> m_command;

	void parse_command() const;

	static void print_help();
};
