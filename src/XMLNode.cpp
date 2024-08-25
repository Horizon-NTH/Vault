#include "XMLNode.h"

#include <utility>

XMLNode::XMLNode(std::string tag):
	m_tag(std::move(tag))
{
}

const std::map<XMLNode::Attribute, XMLNode::Value>& XMLNode::attributes() const
{
	return m_attributes;
}

const std::string& XMLNode::tag() const
{
	return m_tag;
}

const std::vector<std::unique_ptr<XMLNode>>& XMLNode::children() const
{
	return m_children;
}

std::map<XMLNode::Attribute, XMLNode::Value>& XMLNode::attributes()
{
	return m_attributes;
}

std::vector<std::unique_ptr<XMLNode>>& XMLNode::children()
{
	return m_children;
}
