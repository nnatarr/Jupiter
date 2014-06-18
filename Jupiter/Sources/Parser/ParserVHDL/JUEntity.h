
#ifndef __JUENTITY_H__
#define __JUENTITY_H__

#include <QString>
#include <QList>
#include <QStringList>

class JUEntity {
public:
    JUEntity(const QString& name);
    ~JUEntity();

    void addMappedSignals(const QString& componentName, QStringList signal, const QString& label = QString(""));

    struct Port {
        QString name;
        QString mode;
        QString type;

    private:
        Port();
        friend class JUEntity;
    };

    QString name() const { return m_name; }
    QList<Port> portsIn() const { return m_portsIn; }
    QList<Port> portsOut() const { return m_portsOut; }
    QList<JUEntity *> components() const { return m_components; }
    QList<Port> declaredSignals() const { return m_declaredSignals; }

    void addPort(const QString& name, const QString& mode, const QString& type);
    void addPort(Port p);
    static Port createPort(const QString& name, const QString& mode, const QString& type);

    void addComponent(JUEntity *e) { if (!m_components.contains(e)) m_components.append(e); }
    void addDeclaredSignal(Port signal) { m_declaredSignals.append(signal); }

    QString description();
private:
    QString m_name;
    QList<Port> m_portsIn;
    QList<Port> m_portsOut;
    QList<JUEntity *> m_components;
    QList<Port> m_declaredSignals;

    struct MappedSignals {
        QString componentName;
        QStringList signal;
        QString label;
    };

    QList<MappedSignals> m_signalsMap;
};

#endif