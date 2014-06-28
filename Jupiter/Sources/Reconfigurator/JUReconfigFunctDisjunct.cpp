
#include "JUReconfigFunctDisjunct.h"

#include <QStringBuilder>

JUReconfigFunctDisjunct::JUReconfigFunctDisjunct(QString impl)
{
    m_impls.append(QList<QString>() << impl);
    m_varsCount = impl.length();

    m_portsIn = getUsedPorts(m_impls);
    m_lut = generateLUT(impl);

    m_rank = m_portsIn.count();
}

JUReconfigFunctDisjunct::JUReconfigFunctDisjunct(const JUReconfigFunctDisjunct& disjunct)
{
    m_impls.append(disjunct.m_impls);
    m_varsCount = disjunct.m_varsCount;
    m_portsIn.unite(disjunct.m_portsIn);
    m_lut.unite(disjunct.m_lut);
    m_rank = disjunct.m_rank;
}

JUReconfigFunctDisjunct::~JUReconfigFunctDisjunct()
{
}

// ========================================

JUReconfigFunctDisjunct& JUReconfigFunctDisjunct::operator+=(const JUReconfigFunctDisjunct& d)
{
    *this = *this + d;
    return *this;
}

JUReconfigFunctDisjunct JUReconfigFunctDisjunct::operator+(const JUReconfigFunctDisjunct& disjunct)
{
    JUReconfigFunctDisjunct result(*this);

    result.m_impls.append(disjunct.m_impls);
    result.m_portsIn = result.m_portsIn.unite(disjunct.m_portsIn);
    result.m_rank = result.m_portsIn.count();
    result.m_lut = or_LUT(result.m_lut, disjunct.m_lut);

    return result;
}

// ========================================

int JUReconfigFunctDisjunct::rank()
{
    return m_rank;
}

int JUReconfigFunctDisjunct::or_rank(const JUReconfigFunctDisjunct& one, const JUReconfigFunctDisjunct& two)
{
    QSet<int> ports = one.m_portsIn;
    return (ports.unite(two.m_portsIn)).count();
}

// ========================================

QMap<QString, QString> JUReconfigFunctDisjunct::generateEmptyLUT(int varsCount)
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

QMap<QString, QString> JUReconfigFunctDisjunct::generateLUT(QString impl)
{
    QMap<QString, QString> lut = generateEmptyLUT(impl.length());
    for (int i = 0; i < lut.count(); ++i) {
        QString key = lut.keys()[i];
        if (represents(impl, key)) {
            lut[key] = "1";
        }
    }
    return lut;
}

QMap<QString, QString> JUReconfigFunctDisjunct::or_LUT(QMap<QString, QString> lut_one, QMap<QString, QString> lut_two)
{
    QMap<QString, QString> resultLUT;
    for (int i = 0; i < lut_one.count(); ++i) {
        QString key = lut_one.keys()[i];
        if (lut_one[key] == "1" || lut_two[key] == "1") {
            resultLUT[key] = "1";
        } else {
            resultLUT[key] = "0";
        }
    }
    return resultLUT;
}

QSet<int> JUReconfigFunctDisjunct::getUsedPorts(QList<QList<QString>> impl)
{
    QSet<int> ports;
    for (int k = 0; k < impl.count(); ++k) {
        QList<QString> impls = impl[k];
        for (int i = 0; i < impls.count(); ++i) {
            for (int j = 0; j < impls[i].length(); ++j) {
                if (impls[i][j] != 0) {
                    ports.insert(j);
                    if (ports.count() == impls[i].length()) {
                        return ports;
                    }
                }
            }
        }
    }
    return ports;
}

// ========================================

bool JUReconfigFunctDisjunct::represents(QString x, QString y)
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

QString JUReconfigFunctDisjunct::description()
{
    QString desc = "disjunct description:\n";
    desc = desc % "\t|  rank is " % QString::number(m_rank) % "\n";
    desc = desc % "\t|  used ports are " % intListDescription(m_portsIn.toList()) % "\n";
    if (m_impls.count() > 0) {
        desc = desc % "\t|  implicants are " % stringListDescription(m_impls[0]) % "\n";
        for (int i = 1; i < m_impls.count(); ++i) {
            desc = desc % "\t|                 " % stringListDescription(m_impls[i]) % "\n";
        }
    }
    desc = desc % "\t|  LUT is " % m_lut.keys()[0] % " => " % m_lut[m_lut.keys()[0]] % "\n";
    for (int i = 1; i < m_lut.count(); ++i) {
        desc = desc % "\t|         " % m_lut.keys()[i] % " => " % m_lut[m_lut.keys()[i]] % "\n";
    }

    return desc;
}

QString JUReconfigFunctDisjunct::stringListDescription(QList<QString> list)
{
    QString desc;
    if (list.count() > 0) {
        desc = list[0];
    }
    for (int i = 1; i < list.count(); ++i) {
        desc = desc % ", " % list[i];
    }
    return desc;
}

QString JUReconfigFunctDisjunct::intListDescription(QList<int> list)
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