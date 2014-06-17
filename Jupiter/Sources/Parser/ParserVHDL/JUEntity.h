
#ifndef __JUENTITY_H__
#define __JUENTITY_H__

#include <QString>
#include <QList>

class JUEntity {
public:
    JUEntity(const QString& name);
    ~JUEntity();

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

    void addPort(const QString& name, const QString& mode, const QString& type);
    void addPort(Port p);
    static Port createPort(const QString& name, const QString& mode, const QString& type);

    QString description();

private:
    QString m_name;
    QList<JUEntity *> m_useComponents;
    QList<Port> m_portsIn;
    QList<Port> m_portsOut;
};

#endif