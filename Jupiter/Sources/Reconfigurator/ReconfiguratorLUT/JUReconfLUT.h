
#ifndef __JURECONFLUT_H__
#define __JURECONFLUT_H__

#include "JUSchemeErrorLUT.h"
#include "JUProtoLUT.h"
#include "Reconfigurator/JUReconfigFunctDisjunct.h"

#include <QString>
#include <QMap>
#include <QList>

class JUReconfLUT
{
public:
    JUReconfLUT(QList<QString> cmnLut, QList<JUSchemeErrorLUT> errors, int elemsCount, int reservedElemsCount, int elemPortsCount);
    ~JUReconfLUT();

    bool configure();

    QString vhdlDescription() { return m_vhdl; }

private:
    int m_elemPortsCount;
    int m_elemsCount;
    int m_reservedElemsCount;
    QList<JUSchemeErrorLUT> m_errors;
    QList<JUProtoLUT> m_freeElems;
    QList<JUProtoLUT> m_savedElems;
    QList<QString> m_cmnLut;
    QList<JUReconfigFunctDisjunct> m_disjuncts;
    QString m_vhdl;

    void uniteDisjuncts(bool incrementRank = false);
    int* kuhnAlgorithm();
    bool checkMatching(int vertex, QMap<int, QList<int>> *candidates, bool *used, int *matching);

    int maxDisjunctRank();
    int maxLUTAvailPorts();

    QList<JUProtoLUT> sort(QList<JUProtoLUT> list, int low, int high);

    QString vhdlDescriptionInner(QList<JUProtoLUT> *luts);
};

#endif
