#include "File.h"
#include "Base64.h"
#include <fstream>

File::File(std::unique_ptr<Status> status, std::string&& data):
	Node(std::move(status)),
	m_data(data)
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
	os << std::string(indentation, '\t') << "<file name=" << m_status->name << " data=\"" << data() << "\"/>" << std::endl;
}

void File::create(const std::filesystem::path& path) const
{
	const auto full_path = path / m_status->name;
	std::ofstream file(full_path.string(), std::ios::binary);
	if (!file.is_open())
		throw std::ios_base::failure("Failed to create the file: " + full_path.string());

	const auto data = Base64::decode(m_data);
	file.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));

	file.close();
}
