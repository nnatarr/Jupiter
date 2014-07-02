
#ifndef __JUPROTOUBS_H__
#define __JUPROTOUBS_H__

#include "JUSchemeErrorUBS.h"

#include <QString>
#include <QMap>
#include <QSet>

class JUProtoUBS
{
public:
    JUProtoUBS(QString name, JUSchemeErrorUBS error, QString outputName);
    ~JUProtoUBS();

    enum UBSElemType { Full, Two, OneTwo, AndOr };
    UBSElemType type() { return m_type; }

    void mapPortToSignal(int port, const QString& signal);

    QMap<int, QString> portSignalMap() { return m_mapPortSignal; }
    //QMap<QString, int> signalPortMap() { return m_mapSignalPort; }

    int getFirstAvailablePort() {  return m_portsAvailable.values().first(); }
    QList<int> portsAvail() { return m_portsAvailable.values(); }

    QString name() { return m_name; }
    QString outputName() { return m_outputName; }
    void setOutputName(QString n) { m_outputName = n; }

    QList<int> brokenPorts() { return m_brokenPorts.values(); }

private:
    QSet<int> m_portsAvailable;
    QSet<int> m_brokenPorts;
    JUSchemeErrorUBS m_error;
    UBSElemType m_type;
    QString m_name;
    QString m_outputName;
    
    //QMap<QString, int> m_mapSignalPort;
    QMap<int, QString> m_mapPortSignal;


    void processError();

    QString intListDescription(QList<int> list);
    QString mapIntIntDescription(QMap<int, int> map);
    QString mapIntStringDescription(QMap<int, QString> map);
};

#endif