#pragma once

#include <filesystem>

#include "EncryptionManager.h"
#include <optional>

enum class Answer
{
	YES,
	NO,
	ABORT
};

std::optional<EncryptionManager::Password> ask_password_with_confirmation();
Answer ask_confirmation(const std::string& question, Answer defaultAnswer = Answer::YES);
std::filesystem::path get_temp_name(const std::filesystem::path& parentPath);
