#pragma once

#include "Directory.h"
#include <optional>

class VaultManager;

class Vault final : public Directory
{
	friend VaultManager;

public:
	explicit Vault(const std::filesystem::path& file);

	void open(const std::optional<std::filesystem::path>& destination = std::nullopt);
	void close(const std::optional<std::filesystem::path>& destination = std::nullopt, const std::optional<std::string>& extension = std::nullopt, bool encrypt = false);

private:
	std::filesystem::directory_entry m_file;
	bool m_opened;

	void read_from_dir();
	void write_to_dir();
	void read_from_file();
	void write_to_file(bool encrypt) const;
	void extract_from_xml(std::string&& content);

	void write_content(std::ostream& os, size_t indentation) const override;
};
