
#include "JUSchemeErrorUBS.h"

#include <QStringBuilder>

JUSchemeErrorUBS::JUSchemeErrorUBS() : JUSchemeError()
{
    m_errorType = UBSSchemeErrorTypeNone;
    m_innerElementIndex = -1;
}

JUSchemeErrorUBS::~JUSchemeErrorUBS()
{
}

// ========================================

void JUSchemeErrorUBS::setType()
{
    m_elementType = ElementTypeUBS;
}

bool JUSchemeErrorUBS::initError(JUSchemeErrorUBS::UBSSchemeErrorType errorType, int innerElementIndex)
{
    m_errorType = errorType;
    m_innerElementIndex = innerElementIndex;

    return isValid();
}

bool JUSchemeErrorUBS::isValid()
{
    return m_errorType == UBSSchemeErrorTypeNone || (m_errorType != UBSSchemeErrorTypeNone && m_innerElementIndex > -1);
}

QString JUSchemeErrorUBS::description()
{
    QString desc;
    switch (m_errorType) {
    case UBSSchemeErrorTypeBridging:
        desc = "Bridging (" % QString::number(m_innerElementIndex) % ")";
        break;
    case UBSSchemeErrorTypePort:
        desc = "Port (signal " % QString::number(m_innerElementIndex) % ")";
        break;
    case UBSSchemeErrorTypeTransistor:
        desc = "Transistor (" % QString::number(m_innerElementIndex) % ")";
        break;
    case UBSSchemeErrorTypeNone:
    default:
        desc = "None";
    }
    return desc;
}