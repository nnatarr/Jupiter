
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
    m_components.clear();
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

void JUEntity::addMappedSignals(const QString& componentName, QStringList signal, const QString& label)
{
    MappedSignals m;
    m.componentName = componentName;
    m.signal = signal;
    m.label = label;

    m_signalsMap.append(m);

    QString signalsDesc;
    if (signal.count() > 0) {
        signalsDesc = signal[0];
    }
    for (int i = 1; i < signal.count(); ++i) {
        signalsDesc = signalsDesc % ", " % signal[i];
    }
    //JUMLog("added signals map => %s: %s (%s).", (label.isEmpty() ? "<no label>" : Q(label)), Q(componentName), Q(signalsDesc));
}

// ========================================

QString JUEntity::description()
{
    QString desc = QString("entity \"%1\":\n").arg(m_name);

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
    desc = desc % portOutDesc % "\n";
    QString usedComponentsDesc = QString(" | used components => ");
    for (int i = 0; i < m_components.count(); ++i) {
        usedComponentsDesc = usedComponentsDesc % m_components[i]->name() % (i == m_components.count() - 1 ? "" : ", ");
    }
    desc = desc % usedComponentsDesc % "\n";
    QString declaredSignalsDesc = QString(" | declared signals => ");
    for (int i = 0; i < m_declaredSignals.count(); ++i) {
        Port s = m_declaredSignals[i];
        declaredSignalsDesc = declaredSignalsDesc % s.name % ": " % s.type % "; ";
    }
    desc = desc % declaredSignalsDesc % "\n";
    QString signalsMapDesc = QString(" | signals map =>\n");
    for (int i = 0; i < m_signalsMap.count(); ++i) {
        MappedSignals ms = m_signalsMap[i];
        QString signalsDesc;
        if (ms.signal.count() > 0) {
            signalsDesc = ms.signal[0];
        }
        for (int i = 1; i < ms.signal.count(); ++i) {
            signalsDesc = signalsDesc % ", " % ms.signal[i];
        }
        signalsMapDesc = signalsMapDesc % " |                " % (ms.label.isEmpty() ? "<no label>" : ms.label) % ": " % ms.componentName % "(" % signalsDesc % ")\n";
    }
    desc = desc % signalsMapDesc;

    return desc;
}

// PORT =======================================================================

JUEntity::Port::Port()
{

}