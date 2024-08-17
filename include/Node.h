#pragma once

#include <filesystem>

class Node
{
public:
	struct Status
	{
		std::filesystem::path name;

		Status() = default;
		explicit Status(std::filesystem::path name);
		virtual ~Status() = default;
	};

	explicit Node(std::unique_ptr<Status> status);
	virtual ~Node() = default;

	[[nodiscard]] const std::unique_ptr<Status>& status() const;

	virtual void write_content(std::ostream& os, size_t indentation) const = 0;
	virtual void create(const std::filesystem::path& path) const = 0;

protected:
	std::unique_ptr<Status> m_status;
};
