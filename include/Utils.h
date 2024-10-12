#pragma once

#include <filesystem>

#include "EncryptionManager.h"
#include <optional>

std::optional<EncryptionManager::Password> ask_password_with_confirmation();
std::filesystem::path get_temp_name(const std::filesystem::path& parentPath);
