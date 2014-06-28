
#ifndef __JUSCHEMEERRORUBS_H__
#define __JUSCHEMEERRORUBS_H__

#include "Reconfigurator/JUSchemeError.h"

class JUSchemeErrorUBS : public JUSchemeError
{
public:
    JUSchemeErrorUBS();
    virtual ~JUSchemeErrorUBS();

    enum UBSSchemeErrorType { UBSSchemeErrorTypeNone, UBSSchemeErrorTypePort, UBSSchemeErrorTypeTransistor, UBSSchemeErrorTypeBridging };
    bool initError(UBSSchemeErrorType errorType, int innerElementIndex);

    virtual bool isValid();

protected:
    void setType();

private:
    UBSSchemeErrorType m_errorType;
};

#endif