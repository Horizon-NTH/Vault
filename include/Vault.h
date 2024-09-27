#pragma once

#include "Directory.h"
#include <optional>

class Vault final : public Directory
{
public:
	explicit Vault(const std::filesystem::path& file, const std::optional<std::string>& extension = std::nullopt);

	void open(const std::optional<std::filesystem::path>& destination = std::nullopt);
	void close(const std::optional<std::filesystem::path>& destination = std::nullopt);

private:
	struct Status final : Directory::Status
	{
		bool opened{};
		std::optional<std::string> extension;

		Status() = default;
		Status(const std::filesystem::path& name, bool opened, const std::optional<std::string>& extension = std::nullopt);
	};

	std::filesystem::directory_entry m_file;

	void read_from_dir();
	void write_to_dir();
	void read_from_file();
	void write_to_file();
	void remove() const;
	void extract_from_xml(std::string&& content);

	void write_content(std::ostream& os, size_t indentation) const override;
};
