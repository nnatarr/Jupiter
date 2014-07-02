
#ifndef __JURECONFLUT_H__
#define __JURECONFLUT_H__

#include "JUSchemeErrorLUT.h"
#include "JUProtoLUT.h"
#include "Reconfigurator/JUReconfigFunctDisjunct.h"

#include <QString>
#include <QMap>
#include <QList>
#include <QPixmap>
#include <QPainter>

class JUReconfLUT
{
public:
    JUReconfLUT(QList<QString> cmnLut, QList<JUSchemeErrorLUT> errors, int elemsCount, int reservedElemsCount, int elemPortsCount);
    ~JUReconfLUT();

    bool configure();

    QString vhdlDescription() { return m_vhdl; }
    QPixmap pixmapDescription() { return m_pixmap; }

    QString errorMsg() { return m_errorMsg; }

private:
    QString m_errorMsg;
    int m_elemPortsCount;
    int m_elemsCount;
    int m_reservedElemsCount;
    int m_lutElemHeight;
    QList<JUSchemeErrorLUT> m_errors;
    QList<JUProtoLUT> m_freeElems;
    QList<JUProtoLUT> m_savedElems;
    QList<QString> m_cmnLut;
    QList<JUReconfigFunctDisjunct> m_disjuncts;
    QString m_vhdl;
    QPixmap m_pixmap;

    QMap<QString, QColor> m_inputColors;
    QMap<QString, QRect> m_lutRects;
    QMap<QString, QRect> m_inputRects;

    void uniteDisjuncts(bool incrementRank = false);
    int* kuhnAlgorithm();
    bool checkMatching(int vertex, QMap<int, QList<int>> *candidates, bool *used, int *matching);

    int maxDisjunctRank();
    int maxLUTAvailPorts();

    QList<JUProtoLUT> sort(QList<JUProtoLUT> list, int low, int high);

    QString vhdlDescriptionInner(QList<JUProtoLUT> *luts);
    QPixmap pixmapDescriptionInner(QList<JUProtoLUT> *luts);

    void drawInput(int x, int y, QString name, QPainter *p);
    void drawOutput(int x, int y, QPainter *p);
    void drawLUT(int x, int y, JUProtoLUT l, QPainter *p);

    QList<int> sortListInt(QList<int> list, int low, int high);
};

#endif
