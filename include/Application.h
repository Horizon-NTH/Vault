#pragma once

#include <functional>
#include <span>
#include <CLI/CLI.hpp>

class Application
{
public:
	explicit Application(const std::span<const char*>& args);

	int execute();

private:
	CLI::App m_parser;
	std::span<const char*> m_args;

	mutable std::function<void()> m_command;

	void parse_args();
	static void print_version();
};
