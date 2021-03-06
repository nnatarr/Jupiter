
#ifndef __JUPARSER_H__
#define __JUPARSER_H__

#include "JUEntity.h"

class JUParser
{
public:
    virtual QList<JUEntity *> parse(const QString& filePath) = 0;
    virtual bool isErrorSet() = 0;
    virtual QString errorMsg() = 0;
};

#endif