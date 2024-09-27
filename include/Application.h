#pragma once

#include <span>
#include <CLI/CLI.hpp>
#include "VaultManager.h"

class Application
{
public:
	explicit Application(const std::span<const char*>& args, std::unique_ptr<VaultManager> vaultManager = std::make_unique<VaultManager>());
	Application(const Application&) = delete;
	Application(Application&&) = delete;

	int execute();

private:
	CLI::App m_parser;
	std::unique_ptr<VaultManager> m_vaultManager;
	std::span<const char*> m_args;

	void set_args_parsing();
	static void print_version();
};
