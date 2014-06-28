
#ifndef __JUPROTOLUT_H__
#define __JUPROTOLUT_H__

#include "JUSchemeErrorLUT.h"
#include "Reconfigurator/JUReconfigFunctDisjunct.h"

#include <QString>
#include <QMap>
#include <QSet>

class JUProtoLUT
{
public:
    JUProtoLUT(QString name, int portsCount, JUSchemeErrorLUT error, QString outputName);
    ~JUProtoLUT();

    bool setDisjunct(JUReconfigFunctDisjunct disjunct);

    int portsCount() { return m_portsCount; }
    int availablePortsCount() { return m_portsAvailable.count(); }

    int getFirstAvailablePort();
    void connectPortToSignal(int port, QString signalName);

    bool isAvailable() { return !m_isSaved; }

    QString outputName() { return m_outputName; }
    QString name() { return m_name; }

    QMap<int, int> portSignalMap() const { return m_portSignalMap; }
    QMap<int, QString> portNamedSignalMap() const { return m_portNamedSignalMap; }
    QSet<int> brokenPorts() const { return m_brokenPorts; }
    QMap<QString, QString> lut() const { return m_lut; }

    QString description();
    QString vhdlDescription();

private:
    JUReconfigFunctDisjunct m_disjunct;
    int m_portsCount;
    QSet<int> m_portsAvailable;
    QSet<int> m_brokenPorts;
    JUSchemeErrorLUT m_error;
    bool m_isSaved;
    QString m_name;
    QString m_outputName;
    QMap<int, int> m_portSignalMap;
    QMap<int, QString> m_portNamedSignalMap;
    QMap<QString, QString> m_lut;

    void processError();

    QMap<QString, QString> generateEmptyLUT(int varsCount);
    bool represents(QString x, QString y);

    QString intListDescription(QList<int> list);
    QString mapIntIntDescription(QMap<int, int> map);
    QString mapIntStringDescription(QMap<int, QString> map);
};

#endif