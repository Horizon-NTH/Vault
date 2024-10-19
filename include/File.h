#pragma once

#include "Node.h"
#include <vector>

class File final : public Node
{
public:
	File(std::string name, std::filesystem::file_time_type lastWriteTime, std::string data);

	[[nodiscard]] const std::string& data() const;

	static std::vector<uint8_t> read(const std::filesystem::path& path);

private:
	std::string m_data;

	void write_content(pugi::xml_node& parentNode) const override;
	void create(const std::filesystem::path& parentPath) const override;
};
