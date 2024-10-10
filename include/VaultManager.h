#pragma once

#include <filesystem>
#include <optional>

class VaultManager
{
public:
	VaultManager() = default;
	virtual ~VaultManager() = default;

	virtual void open_vault(const std::filesystem::path& vault, const std::optional<std::filesystem::path>& destination);
	virtual void close_vault(const std::filesystem::path& vault, const std::optional<std::filesystem::path>& destination, const std::optional<std::string>& extension);
};
