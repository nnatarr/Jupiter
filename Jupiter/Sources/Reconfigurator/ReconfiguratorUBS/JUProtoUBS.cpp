
#include "JUProtoUBS.h"
#include "Shared/Logger/JULogger.h"


#include <QtCore>

JUProtoUBS::JUProtoUBS(QString name, JUSchemeErrorUBS error, QString outputName)
{
    m_name = name;
    m_error = error;
    m_outputName = outputName;

    processError();
}

JUProtoUBS::~JUProtoUBS()
{
}

// ========================================

void JUProtoUBS::processError()
{
    switch (m_error.type()) {
    case JUSchemeErrorUBS::UBSSchemeErrorTypePort:
        if (m_error.innerElementIndex() == 0) {
            m_type = OneTwo;
            m_portsAvailable << 0 << 2 << 3;
            m_brokenPorts << 1;
        } else {
            m_type = Two;
            m_portsAvailable << 0 << 1;
            m_brokenPorts << 2 << 3;
        }
        break;
    case JUSchemeErrorUBS::UBSSchemeErrorTypeTransistor:
        m_type = OneTwo;
        for (int i = 0; i < 4; ++i) {
            if (m_error.innerElementIndex() != i) {
                m_portsAvailable << i;
            } else {
                m_brokenPorts << i;
            }
        }
        break;
    case JUSchemeErrorUBS::UBSSchemeErrorTypeBridging:
        if (m_error.innerElementIndex() == 1) {
            m_type = AndOr;
            m_portsAvailable << 0 << 1 << 3;
            m_brokenPorts << 2;
        } else {
            m_type = OneTwo;
            if (m_error.innerElementIndex() == 0) {
                m_portsAvailable << 0 << 2 << 3;
                m_brokenPorts << 1;
            } else {
                m_portsAvailable << 0 << 1 << 2;
                m_brokenPorts << 3;
            }
        }
        break;
    case JUSchemeErrorUBS::UBSSchemeErrorTypeNone:
    default:
        m_type = Full;
        m_portsAvailable << 0 << 1 << 2 << 3;
        break;
    }
}

// ========================================

void JUProtoUBS::mapPortToSignal(int port, const QString& signal)
{
    if (m_name == "eubs1") {
        JULog("atata");
    }

    m_mapPortSignal[port] = signal;
    m_portsAvailable.remove(port);
}

// ========================================

QString JUProtoUBS::intListDescription(QList<int> list)
{
    QString desc;
    if (list.count() > 0) {
        desc = QString::number(list[0]);
    }
    for (int i = 1; i < list.count(); ++i) {
        desc = desc % ", " % QString::number(list[i]);
    }
    return desc;
}

QString JUProtoUBS::mapIntIntDescription(QMap<int, int> map)
{
    QString desc;
    if (map.count() > 0) {
        int key = map.keys()[0];
        desc = QString::number(key) % " -> " % QString::number(map[key]);
    }
    for (int i = 1; i < map.count(); ++i) {
        int key = map.keys()[i];
        desc = desc % ", " % QString::number(key) % " -> " % QString::number(map[key]);
    }
    return desc;
}

QString JUProtoUBS::mapIntStringDescription(QMap<int, QString> map)
{
    QString desc;
    if (map.count() > 0) {
        int key = map.keys()[0];
        desc = QString::number(key) % " -> " % map[key];
    }
    for (int i = 1; i < map.count(); ++i) {
        int key = map.keys()[i];
        desc = desc % ", " % QString::number(key) % " -> " % map[key];
    }
    return desc;
}