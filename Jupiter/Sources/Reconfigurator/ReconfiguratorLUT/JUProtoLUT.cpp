
#include "JUProtoLUT.h"
#include "Shared/Logger/JULogger.h"


#include <QtCore>

JUProtoLUT::JUProtoLUT(QString name, int portsCount, JUSchemeErrorLUT error, QString outputName) : 
    m_disjunct("")
{
    m_name = name;
    m_portsCount = portsCount;
    m_error = error;
    m_outputName = outputName;

    m_isSaved = false;

    processError();
}

JUProtoLUT::~JUProtoLUT()
{
}

// ========================================

bool JUProtoLUT::setDisjunct(JUReconfigFunctDisjunct disjunct)
{
    if (m_isSaved) {
        return false;
    }

    m_disjunct = disjunct;
    m_isSaved = true;

    QList<int> dPortsIn = m_disjunct.portsIn().values();
    QList<int> lutAvailPorts = m_portsAvailable.values();
    for (int i = 0; i < dPortsIn.count(); ++i) {
        int port = lutAvailPorts.first();
        m_portSignalMap[port] = dPortsIn[i];
        lutAvailPorts.removeFirst();
        m_portsAvailable.remove(port);
    }

    m_lut = generateEmptyLUT(m_portsCount);
    QString dInputPattern = "";
    QMap<QString, QString> dLut = m_disjunct.lut();
    for (int i = 0; i < dLut.keys()[0].length(); ++i) {
        dInputPattern = dInputPattern % "-";
    }
    for (int i = 0; i < m_lut.count(); ++i) {
        QString input = m_lut.keys()[i];
        QString dInput = dInputPattern;
        for (int j = 0; j < m_portSignalMap.count(); ++j) {
            int port = m_portSignalMap.keys()[j];
            int dPort = m_portSignalMap[port];
            dInput[dPort] = input[port];
        }
        for (int j = 0; j < dLut.count(); ++j) {
            QString dKey = dLut.keys()[j];
            if (represents(dInput, dKey)) {
                m_lut[input] = dLut[dKey];
                break;
            }
        }
    }

    return true;
}

int JUProtoLUT::getFirstAvailablePort()
{
    if (m_portsAvailable.count() > 0) {
        return m_portsAvailable.values()[0];
    } else {
        return -1;
    }
}

void JUProtoLUT::connectPortToSignal(int port, QString signalName)
{
    if (m_portsAvailable.contains(port)) {
        m_portsAvailable.remove(port);

        m_portNamedSignalMap[port] = signalName;

        m_isSaved = true;

        if (m_lut.count() == 0) {
            m_lut = generateEmptyLUT(m_portsCount);
            for (int i = 0; i < m_lut.count(); ++i) {
                QString key = m_lut.keys()[i];
                if (key.contains('1')) {
                    m_lut[key] = "1";
                }
            }
        }

        for (int i = 0; i < m_lut.count(); ++i) {
            QString key = m_lut.keys()[i];
            if (key[port] == '1') {
                m_lut[key] = "1";
            }
        }
    }
}

// ========================================

void JUProtoLUT::processError()
{
    int portIndex = m_error.innerElementIndex();
    switch (m_error.type()) {
    case JUSchemeErrorLUT::LUTSchemeErrorTypePort:
        for (int i = 0; i < m_portsCount; ++i) {
            if (i == portIndex) {
                m_brokenPorts.insert(i);
            } else {
                m_portsAvailable.insert(i);
            }
        }
        break;
    case JUSchemeErrorLUT::LUTSchemeErrorTypeTransistor:
        portIndex = (int)(qLn(m_error.innerElementIndex() + 1) / qLn(2.0));
        for (int i = 0; i < m_portsCount; ++i) {
            if (i == portIndex) {
                m_brokenPorts.insert(i);
            } else {
                m_portsAvailable.insert(i);
            }
        }
        break;
    case JUSchemeErrorLUT::LUTSchemeErrorTypeMultiplexer:
        if (m_error.innerElementIndex() < m_portsCount) {
            for (int i = 0; i < m_portsCount; ++i) {
                if (i == portIndex) {
                    m_brokenPorts.insert(i);
                } else {
                    m_portsAvailable.insert(i);
                }
            }
        }
        else {
            m_portsAvailable.clear();
        }
        break;
    case JUSchemeErrorLUT::LUTSchemeErrorTypeNone:
    default:
        for (int i = 0; i < m_portsCount; ++i) {
            m_portsAvailable.insert(i);
        }
        break;
    }
}

// ========================================

QMap<QString, QString> JUProtoLUT::generateEmptyLUT(int varsCount)
{
    QMap<QString, QString> lut;
    int maxValue = (int)pow(2, varsCount);
    for (int i = 0; i < maxValue; ++i) {
        QString input = QString::number(i, 2);
        while (input.length() < varsCount) {
            input = "0" % input;
        }
        lut[input] = "0";
    }
    return lut;
}

bool JUProtoLUT::represents(QString x, QString y)
{
    if (x.length() != y.length()) {
        return false;
    }

    for (int i = 0; i < x.length(); ++i) {
        if (x[i] != y[i] && x[i] != '-' && y[i] != '-') {
            return false;
        }
    }

    return true;
}

// ========================================

QString JUProtoLUT::vhdlDescription()
{
    QString lutDesc = "entity " % m_name % " is\n";
    lutDesc = lutDesc % "    port (\n";
    lutDesc = lutDesc % "        input: in std_ulogic_vector(" % QString::number(m_portsCount - 1) % " downto 0);\n";
    lutDesc = lutDesc % "        output: out BIT;\n";
    lutDesc = lutDesc % "    );\n";
    lutDesc = lutDesc % "end entity;\n\n";
    lutDesc = lutDesc % "architecture " % m_name % "_arch of " % m_name % " is\n";
    lutDesc = lutDesc % "begin\n";
    lutDesc = lutDesc % "    process (input) is\n";
    lutDesc = lutDesc % "    begin\n";
    lutDesc = lutDesc % "        case input of\n";
    for (int j = 0; j < m_lut.count(); ++j) {
        QString key = m_lut.keys()[j];
        lutDesc = lutDesc % "            when \"" % key % "\" => output <= \"" % m_lut[key] % "\";\n";
    }
    lutDesc = lutDesc % "        end case;\n";
    lutDesc = lutDesc % "    end process;\n";
    lutDesc = lutDesc % "end architecture " % m_name % "_arch;";

    return lutDesc;
}

// ========================================

QString JUProtoLUT::description()
{
    QString desc = "LUT description:\n";
    desc = desc % "\t|  name: " % m_name % "\n";
    desc = desc % "\t|  ports count: " % QString::number(m_portsCount) % "\n";
    desc = desc % "\t|  available ports: " % intListDescription(m_portsAvailable.values()) % "\n";
    desc = desc % "\t|  broken ports: " % intListDescription(m_brokenPorts.values()) % "\n";
    desc = desc % "\t|  is saved: " % QString::number(m_isSaved) % "\n";
    desc = desc % "\t|  output port name: " % m_outputName % "\n";
    desc = desc % "\t|  ports mapping: " % mapIntIntDescription(m_portSignalMap) % "\n";
    desc = desc % "\t|  ports mapping to named signals: " % mapIntStringDescription(m_portNamedSignalMap) % "\n";

    QString lutDesc = QString("\t|  look-up table:\n");
    for (int i = 0; i < m_lut.count(); ++i) {
        QString key = m_lut.keys()[i];
        lutDesc = lutDesc % "\t|                  " % key % " => " % m_lut[key] % "\n";
    }
    desc = desc % lutDesc;
    
    return desc;
}

QString JUProtoLUT::intListDescription(QList<int> list)
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

QString JUProtoLUT::mapIntIntDescription(QMap<int, int> map)
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

QString JUProtoLUT::mapIntStringDescription(QMap<int, QString> map)
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