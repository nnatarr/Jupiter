
#include "JUReconfigurator.h"

#include "Shared/Logger/JULogger.h"
#include "Shared/JUAssert.h"

#include "JUReconfigFunctDisjunct.h"
#include "ReconfiguratorLUT/JUReconfLUT.h"
#include "ReconfiguratorUBS/JUReconfUBS.h"

#define __JUMODULE__ "Reconfigurator"

JUReconfigurator::JUReconfigurator(QList<JUEntity *> *entities, int maxErrorCount, int reservedElementsCount)
{
    JUMLog("ctor.");

    m_entities = entities;
    m_maxErrorCount = maxErrorCount;
    m_reservedElementsCount = reservedElementsCount;

    if (m_maxErrorCount > m_entities->count() + m_reservedElementsCount) {
        m_maxErrorCount = m_entities->count() + m_reservedElementsCount;
    }

    m_elemPortCount = -1;
    for (int i = 0; i < entities->count(); ++i) {
        int cnt = (*entities)[i]->portsIn().count();
        if (cnt > m_elemPortCount) {
            m_elemPortCount = cnt;
        }
    }

    m_mainEntity = NULL;
    m_isValid = false;
    m_schemeType = JUEntity::EntityTypeNone;

    if (checkMainEntity()) {
        JUMLog("main entity obtained: %s.", Q(m_mainEntity->description()));
    } else {
        JUMLog("reconfigurations can not be done.");
    }

    //test();
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
        m_errorMsg = "No entities found.";
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
        m_errorMsg = "Cannot find main entity.";
        JUMLog("cannot find main entity.");
        return false;
    }

    m_isValid = m_mainEntity->validate();
    return m_isValid;
}

// ========================================

QList<QList<JUSchemeError *>> JUReconfigurator::generateErrors()
{
    QList<QList<JUSchemeError *>> errors;

    if (m_schemeType == JUEntity::EntityTypeLUT) {
        int p = pow(2, m_elemPortCount);
        for (int i = 0; i <= m_maxErrorCount; ++i) {
            for (int j = 0; j <= m_maxErrorCount - i; ++j) {
                for (int k = 0; k <= m_maxErrorCount - (i + j); ++k) {
                    QList<JUSchemeError *> error;
                    for (int l = 0; l < i; ++l) {
                        JUSchemeErrorLUT *e = new JUSchemeErrorLUT;
                        e->initError(JUSchemeErrorLUT::LUTSchemeErrorTypePort, rand() % m_elemPortCount);
                        error.append(e);
                    }
                    for (int l = 0; l < j; ++l) {
                        JUSchemeErrorLUT *e = new JUSchemeErrorLUT;
                        e->initError(JUSchemeErrorLUT::LUTSchemeErrorTypeTransistor, rand() % p);
                        error.append(e);
                    }
                    for (int l = 0; l < k; ++l) {
                        JUSchemeErrorLUT *e = new JUSchemeErrorLUT;
                        e->initError(JUSchemeErrorLUT::LUTSchemeErrorTypeMultiplexer, rand() % m_elemPortCount);
                        error.append(e);
                    }
                    if (error.isEmpty()) {
                        continue;
                    }
                    errors.append(error);
                }
            }
        }
    } else {
        for (int i = 0; i <= m_maxErrorCount; ++i) {
            for (int j = 0; j <= m_maxErrorCount - i; ++j) {
                for (int k = 0; k <= m_maxErrorCount - (i + j); ++k) {
                    QList<JUSchemeError *> error;
                    for (int l = 0; l < i; ++l) {
                        JUSchemeErrorUBS *e = new JUSchemeErrorUBS;
                        e->initError(JUSchemeErrorUBS::UBSSchemeErrorTypePort, rand() % 2);
                        error.append(e);
                    }
                    for (int l = 0; l < j; ++l) {
                        JUSchemeErrorUBS *e = new JUSchemeErrorUBS;
                        e->initError(JUSchemeErrorUBS::UBSSchemeErrorTypeTransistor, rand() % 4);
                        error.append(e);
                    }
                    for (int l = 0; l < k; ++l) {
                        JUSchemeErrorUBS *e = new JUSchemeErrorUBS;
                        e->initError(JUSchemeErrorUBS::UBSSchemeErrorTypeBridging, rand() % 3);
                        error.append(e);
                    }
                    if (error.isEmpty()) {
                        continue;
                    }
                    errors.append(error);
                }
            }
        }
    }

    return errors;
}

JUReconfigurator::ConfigurationResult JUReconfigurator::reconfigureForErrors(QList<JUSchemeError *> error)
{
    JUReconfigurator::ConfigurationResult result;
    result.isValid = false;

    if (m_schemeType == JUEntity::EntityTypeLUT) {
        QList<JUSchemeErrorLUT> errors;
        for (int i = 0; i < error.count(); ++i) {
            errors.append(*static_cast<JUSchemeErrorLUT *>(error[i]));
        }
        JUReconfLUT r(m_mainEntity->commonLUT(), errors, (m_mainEntity->type() == JUEntity::EntityTypeLUT ? m_entities->count() : m_mainEntity->singnalsMap().count()), m_reservedElementsCount, m_elemPortCount);
        bool success = r.configure();
        if (success) {
            result.isValid = true;
            result.vhdl = r.vhdlDescription();
            result.pixmap = r.pixmapDescription();
        } else {
            m_errorMsg = r.errorMsg();
        }
    } else {
        QList<JUSchemeErrorUBS> errors;
        for (int i = 0; i < error.count(); ++i) {
            errors.append(*static_cast<JUSchemeErrorUBS *>(error[i]));
        }
        JUReconfUBS r(m_mainEntity, errors, m_mainEntity->singnalsMap().count(), m_reservedElementsCount);
        bool success = r.configure();
        if (success) {
            result.isValid = true;
            result.vhdl = r.vhdl();
            result.pixmap = r.pixmap();
        } else {
            m_errorMsg = r.errorMsg();
        }
    }

    return result;
}