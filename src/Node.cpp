#include <utility>

#include "../include/Node.h"

Node::Status::Status(std::filesystem::path name):
	name(std::move(name))
{
}

Node::Node(std::unique_ptr<Status> status):
	m_status(std::move(status))
{
}

const std::unique_ptr<Node::Status>& Node::status() const
{
	return m_status;
}
