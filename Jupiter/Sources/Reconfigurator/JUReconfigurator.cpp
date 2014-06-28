
#include "JUReconfigurator.h"

#include "Shared/Logger/JULogger.h"
#include "Shared/JUAssert.h"

#include "JUReconfigFunctDisjunct.h"

#define __JUMODULE__ "Reconfigurator"

JUReconfigurator::JUReconfigurator(QList<JUEntity *> *entities, int maxErrorCount, int reservedElementsCount)
{
    JUMLog("ctor.");

    m_entities = entities;
    m_maxErrorCount = maxErrorCount;
    m_reservedElementsCount = reservedElementsCount;

    m_mainEntity = NULL;
    m_isValid = false;
    m_schemeType = JUEntity::EntityTypeNone;

    if (checkMainEntity()) {
        JUMLog("main entity obtained: %s.", Q(m_mainEntity->description()));
    } else {
        JUMLog("reconfigurations can not be done.");
    }

    test();
}

JUReconfigurator::~JUReconfigurator()
{
    JUMLog("dtor.");

    m_entities = NULL;
    m_mainEntity = NULL;
}

// ========================================

bool JUReconfigurator::checkMainEntity()
{
    if (m_entities->count() == 0) {
        JUMLog("no entities found.");
        return false;
    }

    int lutCount = 0;
    int ubsCount = 0;
    QList<JUEntity *> usedEntities;
    for (int i = 0; i < m_entities->count(); ++i) {
        JUEntity *e = (*m_entities)[i];
        if (e->type() == JUEntity::EntityTypeLUT) {
            lutCount++;
        } else if (e->type() == JUEntity::EntityTypeUBS) {
            ubsCount++;
        } else {
            JUAssert(false, "forgot to implement new entity type here.");
        }
        if (e->name() == "inverter" || e->name() == "2band2_or2") {
            continue;
        }
        QList<JUEntity *> components = e->components();
        for (int j = 0; j < components.count(); ++j) {
            if (!usedEntities.contains(components[j])) {
                usedEntities.append(components[j]);
            }
        }
    }

    if (ubsCount == m_entities->count()) {
        m_schemeType = JUEntity::EntityTypeUBS;
    } else if ((lutCount == 1 && m_entities->count() == 1) || (ubsCount == 1 && lutCount == m_entities->count() - 1)) {
        m_schemeType = JUEntity::EntityTypeLUT;
    } else {
        m_schemeType = JUEntity::EntityTypeNone;
        m_isValid = false;
        return false;
    }

    for (int i = 0; i < m_entities->count(); ++i) {
        JUEntity *entity = (*m_entities)[i];
        if (!usedEntities.contains(entity) && !(entity->name() == "inverter" || entity->name() == "2band2_or2")) {
            m_mainEntity = entity;
            break;
        }
    }

    if (!m_mainEntity) {
        JUMLog("cannot find main entity.");
        return false;
    }

    m_isValid = m_mainEntity->validate();
    return m_isValid;
}

// ========================================

void JUReconfigurator::test()
{
    QList<QString> clut = m_mainEntity->commonLUT();

    JUReconfigFunctDisjunct d(clut[0]);
    for (int i = 1; i < clut.count(); ++i) {
        d += JUReconfigFunctDisjunct(clut[i]);
    }
}