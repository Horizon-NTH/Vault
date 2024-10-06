#pragma once

#include "Directory.h"
#include <optional>

class VaultManager;

class Vault final : public Directory
{
	friend VaultManager;

public:
	explicit Vault(const std::string& name, const std::optional<std::filesystem::directory_entry>& from = std::nullopt, const std::optional<std::filesystem::path>& destination = std::nullopt, const std::optional<std::string>& extension = std::nullopt);
	explicit Vault(const std::filesystem::path& file);

	void open(const std::optional<std::filesystem::path>& destination = std::nullopt);
	void close(const std::optional<std::filesystem::path>& destination = std::nullopt);

private:
	std::filesystem::directory_entry m_file;
	bool m_opened;
	std::string m_name;
	std::string m_extension;

	void read_from_dir();
	void write_to_dir();
	void read_from_file();
	void write_to_file();
	void read_settings();
	void write_settings() const;
	void remove() const;
	[[nodiscard]] static bool is_vault(const std::filesystem::path& path);
	void extract_from_xml(std::string&& content);

	void write_content(std::ostream& os, size_t indentation) const override;
};
