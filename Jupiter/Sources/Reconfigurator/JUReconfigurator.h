
#ifndef __JURECONFIGURATOR_H__
#define __JURECONFIGURATOR_H__

#include "Parser/JUEntity.h"
#include "JUSchemeError.h"

#include <QList>
#include <QMap>
#include <QString>
#include <QPixmap>

// LUT proto - lut, norm ports, broken ports, error type

class JUReconfigurator
{
public:
    JUReconfigurator(QList<JUEntity *> *entities, int maxErrorCount, int reservedElementsCount);
    ~JUReconfigurator();

    bool isValid() { return m_isValid; }
    JUEntity::EntityType schemeType() { return m_schemeType; }

    JUEntity* mainEntity() { return m_mainEntity; }

    QList<QList<JUSchemeError *>> generateErrors();

    struct ConfigurationResult {
        bool isValid;
        QString vhdl;
        QPixmap pixmap;
    };
    ConfigurationResult reconfigureForErrors(QList<JUSchemeError *> error);

    void setMaxErrorsCount(int count) { m_maxErrorCount = count; }
    void setReservedElementsCount(int count) { m_reservedElementsCount = count; }

private:
    QList<JUEntity *> *m_entities;
    JUEntity *m_mainEntity;
    bool m_isValid;
    JUEntity::EntityType m_schemeType;
    int m_maxErrorCount;
    int m_reservedElementsCount;
    int m_elemPortCount;

    bool checkMainEntity();

    void test();
};

#endif