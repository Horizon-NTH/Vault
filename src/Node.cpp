#include "Node.h"

#include <utility>

Node::Node(std::filesystem::path name):
	m_name(std::move(name))
{
}
