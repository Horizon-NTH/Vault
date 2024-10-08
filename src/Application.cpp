#include "Application.h"
#include "Vault.h"

Application::Application(const std::span<const char*>& args, std::unique_ptr<VaultManager> vaultManager):
	m_parser("A small, portable file system with encryption capabilities.", "vault"),
	m_vaultManager(std::move(vaultManager)),
	m_args(args)
{
	set_args_parsing();
}

int Application::execute()
{
	try
	{
		m_parser.parse(static_cast<int>(m_args.size()), m_args.data());
	}
	catch (const CLI::CallForVersion&)
	{
		print_version();
	}
	catch (const CLI::ParseError& e)
	{
		return m_parser.exit(e);
	}
	catch (const std::runtime_error& e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

void Application::set_args_parsing()
{
	m_parser.require_subcommand(0, 1)
#if defined(WIN32)
			->allow_windows_style_options()
#endif
			;

	m_parser.add_flag_callback("-v, --version", [] { throw CLI::CallForVersion(); }, "Print the version and exit");

	m_parser.add_subcommand("help", "Print this help message and exit")
	        ->silent()
	        ->parse_complete_callback([] { throw CLI::CallForHelp(); });

	m_parser.add_subcommand("version", "Print the version and exit")
	        ->silent()
	        ->parse_complete_callback([] { throw CLI::CallForVersion(); });

	const auto destination = std::make_shared<std::optional<std::filesystem::path>>();
	const auto name = std::make_shared<std::string>();
	const auto from = std::make_shared<std::optional<std::filesystem::path>>();
	const auto extension = std::make_shared<std::optional<std::string>>();

	const auto create = m_parser.add_subcommand("create", "Create a vault");
	create->add_option("name, -n, --name", *name, "Name of the vault")
	      ->required();
	create->add_option("-f, --from", *from, "Path to the a source directory that will be moved as the vault content")
	      ->check(CLI::ExistingDirectory);
	create->add_option("-d, --destination", *destination, "Path to the destination directory")
	      ->check(CLI::ExistingDirectory);
	create->add_option("-e, --extension", *extension, "Extension of the vault file");
	create->callback([this, name, from, destination, extension] { m_vaultManager->create_vault(*name, from->has_value() ? std::filesystem::directory_entry(from->value()) : std::optional<std::filesystem::directory_entry>{}, *destination, *extension); });

	const auto vaultPath = std::make_shared<std::filesystem::path>();

	const auto open = m_parser.add_subcommand("open", "Open a vault");
	open->add_option("vault, -v, --vault", *vaultPath, "Path to the vault file")
	    ->required()
	    ->check(CLI::ExistingFile);
	open->add_option("destination, -d, --destination", *destination, "Path to the destination directory")
	    ->check(CLI::ExistingDirectory);
	open->callback([this, vaultPath, destination] { m_vaultManager->open_vault(*vaultPath, *destination); });

	const auto close = m_parser.add_subcommand("close", "Close a vault");
	close->add_option("vault, -v, --vault", *vaultPath, "Path to the vault file")
	     ->required()
	     ->check(CLI::ExistingDirectory);
	close->add_option("destination, -d, --destination", *destination, "Path to the destination directory")
	     ->check(CLI::ExistingDirectory);
	close->callback([this, vaultPath, destination] { m_vaultManager->close_vault(*vaultPath, *destination); });
}

void Application::print_version()
{
	std::cout << "vault version " << PROJECT_VERSION << std::endl;
}
