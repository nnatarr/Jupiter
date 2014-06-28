
#ifndef __JUENTITY_H__
#define __JUENTITY_H__

#include <QString>
#include <QList>
#include <QMap>
#include <QStringList>
#include <QSet>

class JUEntity
{
public:
    JUEntity(const QString& name);
    ~JUEntity();
    
    struct MappedSignals {
        QString componentName;
        QStringList signalIn;
        QStringList signalOut;
        QString label;
    };
    void addMappedSignals(const QString& componentName, QStringList signal, const QString& label = QString(""));
    QList<MappedSignals> singnalsMap() const { return m_signalsMap; }

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

    QMap<QString, QString> lut() const { return m_lookupTable; }
    QList<QString> commonLUT() const { return m_commonLookupTable; }

    enum EntityType { EntityTypeNone, EntityTypeLUT, EntityTypeUBS };
    void setType(EntityType type);
    EntityType type() const { return m_type; }

    void addPort(const QString& name, const QString& mode, const QString& type);
    void addPort(Port p);
    static Port createPort(const QString& name, const QString& mode, const QString& type);

    void addComponent(JUEntity *e) { if (!m_components.contains(e)) m_components.append(e); }
    void addDeclaredSignal(Port signal) { m_declaredSignals.append(signal); }

    bool setLUT(QMap<QString, QString> lut);

    QString description();

    bool validate();
    bool getOutput(QString input, QString *output);

    bool hasOutputPort(QString output);

private:
    QString m_name;
    EntityType m_type;
    bool m_isValid;
    bool m_wasValidated;
    QMap<QString, QString> m_lookupTable;
    QList<QString> m_commonLookupTable;
    QList<Port> m_portsIn;
    QList<Port> m_portsOut;
    QList<JUEntity *> m_components;
    QList<Port> m_declaredSignals;

    QList<MappedSignals> m_signalsMap;

    bool generateLookupTable();
    bool generateLUT();
    bool generateUBS();
    bool generateUBSOutputForInput(QString input);
    
    bool generateCommonLookupTable();
    bool join(QString x, QString y, QString *result);
    bool represents(QString x, QString y);
    QString cutChar(QString s, int pos);

    JUEntity *componentByName(const QString& name);

    QString stringListDescription(QStringList list);
    QString intListDescription(QList<int> list);
    QString mapStringStringListDescription(QMap<QString, QString> map);
    QString mapIntStringListDescription(QMap<int, QList<QString>> map);
    QString mapStringIntSetDescription(QMap<QString, QSet<int>> map);
};

#endif