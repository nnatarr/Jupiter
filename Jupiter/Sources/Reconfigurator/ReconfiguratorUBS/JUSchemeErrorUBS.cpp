
#include "JUSchemeErrorUBS.h"

JUSchemeErrorUBS::JUSchemeErrorUBS() : JUSchemeError()
{
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
    return m_errorType != UBSSchemeErrorTypeNone && m_innerElementIndex > -1;
}