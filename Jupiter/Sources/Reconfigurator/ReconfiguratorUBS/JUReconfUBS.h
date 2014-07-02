
#ifndef __JURECONFUBS_H__
#define __JURECONFUBS_H__

#include "JUSchemeErrorUBS.h"
#include "JUProtoUBS.h"

#include "Parser/JUEntity.h"

#include <QString>
#include <QMap>
#include <QList>
#include <QPixmap>
#include <QPainter>

class JUReconfUBS
{
public:
    JUReconfUBS(JUEntity *mainEntity, QList<JUSchemeErrorUBS> errors, int elemsCount, int reservedElemsCount);
    ~JUReconfUBS();

    bool configure();

    QString vhdl() { return m_vhdl; }
    QPixmap pixmap() { return m_pixmap; }

    QString errorMsg() { return m_errorMsg; }

private:
    QString m_errorMsg;
    int m_elemPortsCount;
    int m_elemsCount;
    int m_reservedElemsCount;
    JUEntity *m_mainEntity;
    int m_ubsElemHeight;
    QList<JUSchemeErrorUBS> m_errors;
    QString m_vhdl;
    QPixmap m_pixmap;

    QList<JUProtoUBS> m_mapped;
    QList<JUProtoUBS> m_broken;
    QList<JUEntity::MappedSignals> m_notMappedEntitySignals;

    QMap<QString, QColor> m_inputColors;
    QMap<QString, QRect> m_ubsRects;
    QMap<QString, QRect> m_inputRects;

    struct ThreeInt {
        int type;
        int two;
        int andor;
        int onetwo;
        int self;
    };

    QString vhdlDescriptionInner(QList<JUProtoUBS> *luts);
    QPixmap pixmapDescriptionInner(QList<JUProtoUBS> *luts);

    void drawInput(int x, int y, QString name, QPainter *p);
    void drawOutput(int x, int y, QPainter *p);
    void drawUBS(int x, int y, JUProtoUBS l, QPainter *p);

    QList<int> sortListInt(QList<int> list, int low, int high);
};

#endif
