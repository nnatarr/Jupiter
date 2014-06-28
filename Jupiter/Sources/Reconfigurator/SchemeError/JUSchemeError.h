
#ifndef __JUSCHEMEERROR_H__
#define __JUSCHEMEERROR_H__

class JUSchemeError
{
public:
    JUSchemeError();
    virtual ~JUSchemeError();

    enum ElementType { ElementTypeNone, ElementTypeUBS, ElementTypeLUT };

    virtual bool isValid();

protected:
    ElementType m_elementType;
    int m_innerElementIndex;

    virtual void setType();
};

#endif