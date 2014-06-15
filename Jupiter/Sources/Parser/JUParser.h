
#ifndef __JUPARSER_H__
#define __JUPARSER_H__

#include "JUSchemeTree.h"

class JUParser
{
public:
    virtual JUSchemeTree* parse(const QString& filePath) = 0;
};

#endif