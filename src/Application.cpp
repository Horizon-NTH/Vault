#include "Application.h"

#include <iostream>

#include "Vault.h"

#include <stdexcept>
#include <string>

Application::Application(const std::span<const char*>& args):
	m_parser(args)
{
	parse_command();
}

void Application::execute() const
{
	try
	{
		if (m_command)
			m_command();
		else
			throw std::runtime_error("No command to execute");
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error: " << e.what() << '\n';
	}
}

void Application::parse_command() const
{
	if (auto [command, options] = std::make_pair(m_parser.command(), m_parser.options()); command == "help")
	{
		m_command = [this] { print_help(); };
	}
	else if (command == "open")
	{
		const auto vault = options["vault"],
				destination = options["destination"];
		if (!vault)
			throw std::runtime_error("Vault is required");
		m_command = [=] { Vault::open(*vault, destination); };
	}
	else if (command == "close")
	{
		const auto vault = options["vault"],
				destination = options["destination"],
				extension = options["extension"];
		if (!vault)
			throw std::runtime_error("Vault is required");
		m_command = [=] { Vault::close(*vault, destination, static_cast<std::optional<std::string>>(extension)); };
	}
	else
		throw std::runtime_error("Unknown command " + std::string(command));
}

void Application::print_help()
{
	std::cout << "Usage:\n"
			<< "  vault <command> [options]\n\n"
			<< "Commands:\n"
			<< "  help              Display this help message.\n"
			<< "  open              Open a vault.\n"
			<< "  close             Close a vault.\n\n"
			<< "Options:\n"
			<< "  -v, --vault <path>            Path to the vault (required).\n"
			<< "  -d, --destination <path>      (Optional) Destination path.\n"
			<< "  -e, --extension <ext>         (Optional) File extension to use when closing the vault.\n\n"
			<< "Examples:\n"
			<< "  vault open -v /path/to/vault -d /output/path\n"
			<< "  vault close --vault=/path/to/vault --extension=.vlt\n\n";
}
