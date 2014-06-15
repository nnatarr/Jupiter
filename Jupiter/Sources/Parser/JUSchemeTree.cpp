
#include "JUSchemeTree.h"
#include "Shared/Logger/JULogger.h"

#define __JUMODULE__ "SchemeTree"

JUSchemeTree::JUSchemeTree()
{
    JUMLog("tree ctor {%p}.", this);
}

JUSchemeTree::~JUSchemeTree()
{
    JUMLog("tree dtor {%p}.", this);

    for (int i = 0; i < m_nodes.count(); ++i) {
        JUSchemeNode *node = m_nodes[i];
        delete node;
    }
    m_nodes.clear();
}

// ========================================

JUSchemeNode* JUSchemeTree::createNode(QString name, JUSchemeNode::SchemeNodeType type)
{
    if (name.isEmpty()) {
        return NULL;
    }

    JUSchemeNode *node = new JUSchemeNode(name, type);
    m_nodes.append(node);

    return node;
}

bool JUSchemeTree::addLinkFromNodeToNode(JUSchemeNode *nodeFrom, JUSchemePort *portFrom, JUSchemeNode *nodeTo, JUSchemePort *portTo)
{
    if (nodeFrom == NULL || portFrom == NULL || nodeTo == NULL || portTo == NULL) {
        return false;
    }

    if (portFrom->type() != JUSchemePort::SchemePortTypeOut) {
        JUMLog("can not send signal from incoming port.");
        return false;
    }

    JUSchemeNode::Link link;
    link.portFrom = portFrom;
    link.nodeTo = nodeTo;
    link.portTo = portTo;

    return nodeFrom->addLinkToNode(portFrom, nodeTo, portTo);
}

// NODE =======================================================================

JUSchemeNode::JUSchemeNode(QString name, SchemeNodeType type) :
    m_links(), m_ports()
{
    JUMLog("node ctor {%p}.", this);

    m_name = name;
    m_type = type;
}

JUSchemeNode::~JUSchemeNode()
{
    JUMLog("node dtor {%p}.", this);

    for (int i = 0; i < m_links.count(); ++i) {
        JUSchemeNode::Link *link = m_links[i];
        delete link;
    }
    m_links.clear();

    for (int i = 0; i < m_ports.count(); ++i) {
        JUSchemePort *port = m_ports[i];
        delete port;
    }
    m_ports.clear();
}

// ========================================

JUSchemePort* JUSchemeNode::addPort(QString name, JUSchemePort::SchemePortType type)
{
    if (name.isEmpty()) {
        return NULL;
    }

    JUSchemePort *port = new JUSchemePort(name, type);
    m_ports.append(port);

    return port;
}

bool JUSchemeNode::addLinkToNode(JUSchemePort *portFrom, JUSchemeNode *nodeTo, JUSchemePort *portTo)
{
    if (portFrom == NULL || nodeTo == NULL || portTo == NULL) {
        return false;
    }

    JUSchemeNode::Link *link = new JUSchemeNode::Link();
    link->portFrom = portFrom;
    link->nodeTo = nodeTo;
    link->portTo = portTo;

    m_links.append(link);
    return true;
}

// PORT =======================================================================

JUSchemePort::JUSchemePort(QString name, SchemePortType type)
{
    JUMLog("port ctor {%p}.", this);

    m_name = name;
    m_type = type;
}

JUSchemePort::~JUSchemePort()
{
    JUMLog("port dtor {%p}.", this);
}

// ========================================

bool JUSchemePort::setValueType(JUSchemePort::PortValueType valueType)
{
    m_value = valueType;
    return true;
}