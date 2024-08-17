#pragma once

#include "Directory.h"

class Vault final : public Directory
{
public:
	struct Status final : Directory::Status
	{
		bool opened{};
		std::optional<std::string> extension;

		Status() = default;
		Status(const std::filesystem::path& name, bool opened, const std::optional<std::string>& extension = std::nullopt);
	};

	explicit Vault(const std::filesystem::directory_entry& file, const std::optional<std::string>& extension = std::nullopt);

	static void open(const std::filesystem::path& path, const std::optional<std::filesystem::path>& destination = std::nullopt);
	static void close(const std::filesystem::path& path, const std::optional<std::filesystem::path>& destination = std::nullopt, const std::optional<std::string>& extension = std::nullopt);

private:
	std::filesystem::directory_entry m_file;

	void read_from_dir();
	void write_to_dir() const;
	void read_from_file();
	void write_to_file() const;
	void remove() const;

	void write_content(std::ostream& os, size_t indentation) const override;
	void parse_tokens(std::vector<std::string>& tokens);

	static std::vector<std::string> tokenize(std::string&& content);
};
