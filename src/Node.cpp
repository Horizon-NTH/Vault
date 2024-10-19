#include "Node.h"

#include <utility>

Node::Node(std::string name, const std::filesystem::file_time_type lastWriteTime):
	m_name(std::move(name)),
	m_lastWriteTime(lastWriteTime)
{
}
