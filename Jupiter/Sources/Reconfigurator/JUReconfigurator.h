
#ifndef __JURECONFIGURATOR_H__
#define __JURECONFIGURATOR_H__

#include "Parser/JUEntity.h"

#include <QList>
#include <QMap>

// LUT proto - lut, norm ports, broken ports, error type

class JUReconfigurator
{
public:
    JUReconfigurator(QList<JUEntity *> *entities, int maxErrorCount, int reservedElementsCount);
    ~JUReconfigurator();

    bool isValid() { return m_isValid; }
    JUEntity::EntityType schemeType() { return m_schemeType; }

private:
    QList<JUEntity *> *m_entities;
    JUEntity *m_mainEntity;
    bool m_isValid;
    JUEntity::EntityType m_schemeType;
    int m_maxErrorCount;
    int m_reservedElementsCount;

    bool checkMainEntity();

    void test();
};

#endif