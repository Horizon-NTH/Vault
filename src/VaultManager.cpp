#include "../include/VaultManager.h"
#include "Vault.h"

void VaultManager::open_vault(const std::filesystem::path& vault, const std::optional<std::filesystem::path>& destination)
{
	Vault vault_obj(vault);
	vault_obj.open(destination);
}

void VaultManager::close_vault(const std::filesystem::path& vault, const std::optional<std::filesystem::path>& destination, const std::optional<std::string>& extension, const bool compress, const bool encrypt)
{
	Vault vault_obj(vault);
	vault_obj.close(destination, extension, compress, encrypt);
}
