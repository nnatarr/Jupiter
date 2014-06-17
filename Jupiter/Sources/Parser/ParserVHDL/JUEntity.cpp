
#include "JUEntity.h"
#include "Shared/Logger/JULogger.h"

#include <QStringBuilder>

#define __JUMODULE__ "Entity"

JUEntity::JUEntity(const QString& name)
{
    JUMLog("ctor {%p}.", this);

    m_name = name;
}

JUEntity::~JUEntity()
{
    JUMLog("dtor {%p}.", this);

    m_portsIn.clear();
    m_portsOut.clear();
    m_useComponents.clear();
}

// ========================================

void JUEntity::addPort(const QString& name, const QString& mode, const QString& type)
{
    addPort(JUEntity::createPort(name, mode, type));
}

JUEntity::Port JUEntity::createPort(const QString& name, const QString& mode, const QString& type)
{
    Port p;
    p.name = name;
    p.mode = mode;
    p.type = type;
    return p;
}

void JUEntity::addPort(Port p)
{
    if (p.mode == "in") {
        m_portsIn.append(p);
    } else if (p.mode == "out") {
        m_portsOut.append(p);
    }
}

// ========================================

QString JUEntity::description()
{
    QString desc = QString("Entity %1:\n").arg(m_name);

    QString portInDesc = QString(" | ports in => ");
    for (int i = 0; i < m_portsIn.count(); ++i) {
        Port p = m_portsIn[i];
        portInDesc = portInDesc % p.name % ": " % p.mode % " " % p.type % "; ";
    }
    desc = desc % portInDesc % "\n";
    QString portOutDesc = QString(" | ports out => ");
    for (int i = 0; i < m_portsOut.count(); ++i) {
        Port p = m_portsOut[i];
        portOutDesc = portOutDesc % p.name % ": " % p.mode % " " % p.type;
    }
    desc = desc % portOutDesc;

    return desc;
}

// PORT =======================================================================

JUEntity::Port::Port()
{

}