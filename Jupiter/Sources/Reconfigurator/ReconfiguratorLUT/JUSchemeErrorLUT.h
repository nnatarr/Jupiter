
#ifndef __JUSCHEMEERRORLUT_H__
#define __JUSCHEMEERRORLUT_H__

#include "Reconfigurator/JUSchemeError.h"

class JUSchemeErrorLUT : public JUSchemeError
{
public:
    JUSchemeErrorLUT();
    virtual ~JUSchemeErrorLUT();

    enum LUTSchemeErrorType { LUTSchemeErrorTypeNone, LUTSchemeErrorTypePort, LUTSchemeErrorTypeTransistor, LUTSchemeErrorTypeMultiplexer };
    bool initError(LUTSchemeErrorType errorType, int innerElementIndex);

    LUTSchemeErrorType type() { return m_errorType; }

    virtual bool isValid();

protected:
    void setType();

private:
    LUTSchemeErrorType m_errorType;
};

#endif