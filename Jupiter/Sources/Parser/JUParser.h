
#ifndef __JUPARSER_H__
#define __JUPARSER_H__

#include "JUSchemeTree.h"

class JUParser
{
public:
    virtual JUSchemeTree* parse() = 0;
};

#endif