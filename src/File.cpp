#include "File.h"
#include <botan/base64.h>
#include <fstream>
#include <utility>

File::File(std::filesystem::path name, std::string&& data):
	Node(std::move(name)),
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

void File::write_content(std::ostream& os, const size_t indentation) const
{
	os << std::string(indentation, '\t') << "<file name=\"" << m_name.string() << "\" data=\"" << data() << "\"/>" << std::endl;
}

void File::create(const std::filesystem::path& parentPath) const
{
	const auto full_path = parentPath / m_name;
	std::ofstream file(full_path.string(), std::ios::binary);
	if (!file.is_open())
		throw std::ios_base::failure("Failed to create the file: " + full_path.string());

	const auto data = Botan::base64_decode(m_data);
	file.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
}
