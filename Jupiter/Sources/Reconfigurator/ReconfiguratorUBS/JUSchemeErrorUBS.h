
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

    UBSSchemeErrorType type() { return m_errorType; }

    virtual bool isValid();
    virtual QString description();

protected:
    void setType();

private:
    UBSSchemeErrorType m_errorType;
};

#endif