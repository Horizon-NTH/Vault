#include "Utils.h"
#include <iostream>

#ifdef _WIN32
#include <conio.h>
#else
	#include <termios.h>
	#include <unistd.h>
#endif

namespace
{
	EncryptionManager::Password get_hidden_input()
	{
		EncryptionManager::Password password;

#ifdef _WIN32
		char ch;
		while ((ch = static_cast<char>(_getch())) != '\r')
		{
			if (ch == '\b')
			{
				if (!password.empty())
					password.pop_back();
			}
			else
			{
				password.push_back(ch);
			}
		}
#else
		termios oldt, newt;
		tcgetattr(STDIN_FILENO, &oldt);
		newt = oldt;
		newt.c_lflag &= ~ECHO;
		tcsetattr(STDIN_FILENO, TCSANOW, &newt);

		std::getline(std::cin, password);

		tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
#endif

		std::cout << std::endl;
		return password;
	}
}

std::optional<EncryptionManager::Password> ask_password_with_confirmation()
{
	std::cout << "Enter password: ";
	EncryptionManager::Password password = get_hidden_input();

	std::cout << "Confirm password: ";
	if (const EncryptionManager::Password confirm_password = get_hidden_input(); password == confirm_password)
		return password;
	return std::nullopt;
}

Answer ask_confirmation(const std::string& question, const Answer defaultAnswer)
{
	if (defaultAnswer == Answer::ABORT)
		throw std::invalid_argument("The default answer cannot be ABORT");
	std::cout << question << (defaultAnswer == Answer::YES ? " [Y/n] " : " [y/N] ");
	std::string answer;
	std::getline(std::cin, answer);
	if (answer.empty())
		return defaultAnswer;
	std::ranges::transform(answer, answer.begin(), tolower);
	if (answer == "y" || answer == "yes")
		return Answer::YES;
	if (answer == "n" || answer == "no")
		return Answer::NO;
	return Answer::ABORT;
}

std::filesystem::path get_temp_name(const std::filesystem::path& parentPath)
{
	std::filesystem::path filePath;
	int counter = 0;
	do
	{
		filePath = parentPath / ("temp" + std::to_string(counter));
		++counter;
	} while (exists(filePath));
	return filePath;
}
