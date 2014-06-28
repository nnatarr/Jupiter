
#include "JUSchemeError.h"

JUSchemeError::JUSchemeError()
{
    setType();
    m_innerElementIndex = -1;
}

JUSchemeError::~JUSchemeError()
{
}

// ========================================

void JUSchemeError::setType()
{
    m_elementType = ElementTypeNone;
}

bool JUSchemeError::isValid()
{
    return false;
}

QString JUSchemeError::description()
{
    return "";
}