#include "Application.h"

#include "Utils.h"

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
		if (m_args.size() == 1)
			throw CLI::CallForHelp();
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
	catch (const std::exception& e)
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
	const auto extension = std::make_shared<std::optional<std::string>>();
	const auto vaultPath = std::make_shared<std::filesystem::path>();
	const auto encrypt = std::make_shared<bool>(false);
	const auto compress = std::make_shared<bool>(false);

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
	close->add_option("extension, -e, --extension", *extension, "Extension of the vault file");
	close->add_flag("-E, --encrypt", *encrypt, "Encrypt the vault file, you will be prompted for a password");
	close->add_flag("-C, --compress", *compress, "Compress the vault file");
	close->callback([this, vaultPath, destination, extension, encrypt, compress]
		{
			if (constexpr std::array args = {"-v", "--vault", "-d", "--destination", "-E", "--encrypt", "-C", "--compress"}; extension->has_value() && std::ranges::find(args, extension->value()) != args.end())
			{
				if (const auto answer = ask_confirmation("Your are trying to set the extension as " + extension->value() + " but it is a flag.\nAre you sure you want to continue?", Answer::NO); answer != Answer::YES)
					return;
			}
			m_vaultManager->close_vault(*vaultPath, *destination, *extension, *compress, *encrypt);
		});
}

void Application::print_version()
{
	std::cout << "vault version " << PROJECT_VERSION << std::endl;
}
