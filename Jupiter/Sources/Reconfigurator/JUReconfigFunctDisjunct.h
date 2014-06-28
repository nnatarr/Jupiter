
#ifndef __JURECONFIGFUNCTDISJUNCT_H__
#define __JURECONFIGFUNCTDISJUNCT_H__

#include <QList>
#include <QMap>
#include <QString>
#include <QSet>

class JUReconfigFunctDisjunct
{
public:
    JUReconfigFunctDisjunct(QString impl);
    JUReconfigFunctDisjunct(const JUReconfigFunctDisjunct& disjunct);
    ~JUReconfigFunctDisjunct();

    int rank();
    static int or_rank(const JUReconfigFunctDisjunct& one, const JUReconfigFunctDisjunct& two);

    QMap<QString, QString> lut() const { return m_lut; }

    JUReconfigFunctDisjunct& operator+=(const JUReconfigFunctDisjunct& d);
    JUReconfigFunctDisjunct operator+(const JUReconfigFunctDisjunct& d);

    QString description();

private:
    int m_rank;
    int m_varsCount;
    QList<QList<QString>> m_impls;
    QMap<QString, QString> m_lut;
    QSet<int> m_portsIn;
    QString m_portOut;

    QMap<QString, QString> generateEmptyLUT(int varsCount);
    QMap<QString, QString> generateLUT(QString impl);
    QMap<QString, QString> or_LUT(QMap<QString, QString> lut_one, QMap<QString, QString> lut_two);

    static QSet<int> getUsedPorts(QList<QList<QString>> impl);

    bool represents(QString x, QString y);

    QString stringListDescription(QList<QString> list);
    QString intListDescription(QList<int> list);
};

#endif