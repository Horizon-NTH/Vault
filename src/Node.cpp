#include "Node.h"

#include <utility>

Node::Node(std::string name):
	m_name(std::move(name))
{
}
