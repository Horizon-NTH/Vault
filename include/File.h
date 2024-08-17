#pragma once

#include "Node.h"

class File final : public Node
{
public:
	File(std::unique_ptr<Status> status, std::string&& data);

	[[nodiscard]] const std::string& data() const;

	static std::vector<uint8_t> read(const std::filesystem::path& path);

private:
	std::string m_data;

	void write_content(std::ostream& os, size_t indentation) const override;
	void create(const std::filesystem::path& path) const override;
};
