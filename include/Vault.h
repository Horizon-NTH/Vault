#pragma once

#include "Directory.h"
#include <optional>

class Vault final : public Directory
{
public:
	static void open(const std::filesystem::path& path, const std::optional<std::filesystem::path>& destination = std::nullopt);
	static void close(const std::filesystem::path& path, const std::optional<std::filesystem::path>& destination = std::nullopt, const std::optional<std::string>& extension = std::nullopt);

private:
	struct Status final : Directory::Status
	{
		bool opened{};
		std::optional<std::string> extension;

		Status() = default;
		Status(const std::filesystem::path& name, bool opened, const std::optional<std::string>& extension = std::nullopt);
	};

	std::filesystem::directory_entry m_file;

	explicit Vault(const std::filesystem::directory_entry& file, const std::optional<std::string>& extension = std::nullopt);

	void read_from_dir();
	void write_to_dir() const;
	void read_from_file();
	void write_to_file() const;
	void remove() const;
	void extract_from_xml(std::string&& content);

	void write_content(std::ostream& os, size_t indentation) const override;
};
