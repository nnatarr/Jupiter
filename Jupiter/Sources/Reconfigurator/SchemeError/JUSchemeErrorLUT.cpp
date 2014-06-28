
#include "JUSchemeErrorLUT.h"

JUSchemeErrorLUT::JUSchemeErrorLUT() : JUSchemeError()
{
    m_errorType = LUTSchemeErrorTypeNone;
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