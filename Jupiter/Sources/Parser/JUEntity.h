
#ifndef __JUENTITY_H__
#define __JUENTITY_H__

#include <QString>
#include <QList>
#include <QMap>
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

    enum EntityType { EntityTypeNone, EntityTypeLUT, EntityTypeUBS };
    void setType(EntityType type);
    EntityType type() const { return m_type; }

    void addPort(const QString& name, const QString& mode, const QString& type);
    void addPort(Port p);
    static Port createPort(const QString& name, const QString& mode, const QString& type);

    void addComponent(JUEntity *e) { if (!m_components.contains(e)) m_components.append(e); }
    void addDeclaredSignal(Port signal) { m_declaredSignals.append(signal); }

    QString description();

    bool validate();
    bool getOutput(QString input, QString *output);

private:
    QString m_name;
    EntityType m_type;
    bool m_isValid;
    QMap<QString, QString> m_lookupTable;
    QList<Port> m_portsIn;
    QList<Port> m_portsOut;
    QList<JUEntity *> m_components;
    QList<Port> m_declaredSignals;

    struct MappedSignals {
        QString componentName;
        QStringList signalIn;
        QStringList signalOut;
        QString label;
    };

    QList<MappedSignals> m_signalsMap;

    bool generateLookupTable();
    bool generateLUT();
    bool generateUBS();
    bool generateUBSOutputForInput(QString input);

    JUEntity *componentByName(const QString& name);

    QString stringListDescription(QStringList list);
    QString mapStringStringListDescription(QMap<QString, QString> map);
};

#endif