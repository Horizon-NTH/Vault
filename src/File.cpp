#include "File.h"
#include <botan/base64.h>
#include <fstream>
#include <utility>
#include <format>
#include <chrono>

File::File(std::string name, const std::filesystem::file_time_type lastWriteTime, std::string data):
	Node(std::move(name), lastWriteTime),
	m_data(std::move(data))
{
}

const std::string& File::data() const
{
	return m_data;
}

std::vector<uint8_t> File::read(const std::filesystem::path& path)
{
	std::ifstream file(path.string(), std::ios::binary | std::ios::ate);

	if (!file.is_open())
		throw std::ios_base::failure("Failed to open the file: " + path.string());

	const std::streamsize fileSize = file.tellg();
	if (fileSize < 0)
		throw std::ios_base::failure("Failed to get " + path.string() + " size.");

	std::vector<uint8_t> fileData(fileSize);
	file.seekg(0, std::ios::beg);
	if (!file.read(reinterpret_cast<char*>(fileData.data()), fileSize))
		throw std::ios_base::failure("Failed to read " + path.string() + " data.");

	file.close();

	return std::move(fileData);
}

void File::write_content(pugi::xml_node& parentNode) const
{
	auto node = parentNode.append_child("file");
	if (!node)
		throw std::runtime_error("Failed to create the XML node");
	node.append_attribute("name").set_value(m_name.c_str());
	node.append_attribute("data").set_value(m_data.c_str());
	node.append_attribute("lastWriteTime").set_value(std::format("{}", m_lastWriteTime).c_str());
}

void File::create(const std::filesystem::path& parentPath) const
{
	const auto full_path = parentPath / m_name;
	std::ofstream file(full_path.string(), std::ios::binary);
	if (!file.is_open())
		throw std::ios_base::failure("Failed to create the file: " + full_path.string());

	const auto data = Botan::base64_decode(m_data);
	file.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
	file.close();
	last_write_time(full_path, m_lastWriteTime);
}
