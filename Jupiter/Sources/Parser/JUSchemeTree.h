
#ifndef __JUSCHEMETREE_H__
#define __JUSCHEMETREE_H__

#include <QString>
#include <QList>

class JUSchemePort
{
public:
    enum SchemePortType { SchemePortTypeIn, SchemePortTypeOut };

    SchemePortType type() { return m_type; }
    QString name() const { return m_name; }

private:
    JUSchemePort(QString name, SchemePortType type);
    ~JUSchemePort();

    QString m_name;
    SchemePortType m_type;

    friend class JUSchemeNode;
    friend class JUSchemeTree;
};

// =======================================================================

class JUSchemeNode
{
public:
    enum SchemeNodeType { SchemeNodeTypeLUT, SchemeNodeTypeFPT };
    struct Link {
        JUSchemePort *portFrom;
        JUSchemeNode *nodeTo;
        JUSchemePort *portTo;
    };

    QString name() const { return m_name; }
    SchemeNodeType type() { return m_type; }

    JUSchemePort* addPort(QString name, JUSchemePort::SchemePortType type);

private:
    QString m_name;
    SchemeNodeType m_type;
    QList<JUSchemeNode::Link *> m_links;
    QList<JUSchemePort *> m_ports;

    JUSchemeNode(QString name, SchemeNodeType type);
    ~JUSchemeNode();
    
    bool addLinkToNode(JUSchemePort *portFrom, JUSchemeNode *nodeTo, JUSchemePort *portTo);

    friend class JUSchemeTree;
};

// =======================================================================

class JUSchemeTree
{
public:
    JUSchemeTree();
    ~JUSchemeTree();

    JUSchemeNode* createNode(QString name, JUSchemeNode::SchemeNodeType type);
    
    bool addLinkFromNodeToNode(JUSchemeNode *nodeFrom, JUSchemePort *portFrom, JUSchemeNode *nodeTo, JUSchemePort *portTo);

private:
    QList<JUSchemeNode *> m_nodes;
};

#endif