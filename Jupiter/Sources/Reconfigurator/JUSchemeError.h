
#ifndef __JUSCHEMEERROR_H__
#define __JUSCHEMEERROR_H__

#include <QString>

class JUSchemeError
{
public:
    JUSchemeError();
    virtual ~JUSchemeError();

    enum ElementType { ElementTypeNone, ElementTypeUBS, ElementTypeLUT };
    int innerElementIndex() { return m_innerElementIndex; }

    virtual bool isValid();
    virtual QString description();

protected:
    ElementType m_elementType;
    int m_innerElementIndex;

    virtual void setType();
};

#endif