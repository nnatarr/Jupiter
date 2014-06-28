
#ifndef __JUSCHEMEERRORLUT_H__
#define __JUSCHEMEERRORLUT_H__

#include "JUSchemeError.h"

class JUSchemeErrorLUT : public JUSchemeError
{
public:
    JUSchemeErrorLUT();
    virtual ~JUSchemeErrorLUT();

    enum LUTSchemeErrorType { LUTSchemeErrorTypeNone, LUTSchemeErrorTypePort, LUTSchemeErrorTypeTransistor, LUTSchemeErrorTypeMultiplexer };
    bool initError(LUTSchemeErrorType errorType, int innerElementIndex);

    virtual bool isValid();

protected:
    void setType();

private:
    LUTSchemeErrorType m_errorType;
};

#endif