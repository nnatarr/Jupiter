
#include "JUSchemeErrorLUT.h"

#include <QStringBuilder>

JUSchemeErrorLUT::JUSchemeErrorLUT() : JUSchemeError()
{
    m_errorType = LUTSchemeErrorTypeNone;
    m_innerElementIndex = -1;
}

JUSchemeErrorLUT::~JUSchemeErrorLUT()
{
}

// ========================================

void JUSchemeErrorLUT::setType()
{
    m_elementType = ElementTypeLUT;
}

bool JUSchemeErrorLUT::initError(JUSchemeErrorLUT::LUTSchemeErrorType errorType, int innerElementIndex)
{
    m_errorType = errorType;
    m_innerElementIndex = innerElementIndex;

    return isValid();
}

bool JUSchemeErrorLUT::isValid()
{
    return m_errorType != LUTSchemeErrorTypeNone && m_innerElementIndex > -1;
}

QString JUSchemeErrorLUT::description()
{
    QString desc;
    switch (m_errorType) {
    case LUTSchemeErrorTypePort:
        desc = "Port (" % QString::number(m_innerElementIndex) % ")";
        break;
    case LUTSchemeErrorTypeTransistor:
        desc = "Transistor (" % QString::number(m_innerElementIndex) % ")";
        break;
    case LUTSchemeErrorTypeMultiplexer:
        desc = "Multiplexer(" % QString::number(m_innerElementIndex) % ")";
        break;
    case LUTSchemeErrorTypeNone:
    default:
        desc = "None";
    }
    return desc;
}