
#include "JUParserVHDL.h"
#include "Shared/Logger/JULogger.h"

#define __JUMODULE__ "ParserVHDL"

JUParserVHDL::JUParserVHDL() : JUParser()
{
    JUMLog("ctor {%p}.", this);
}

JUParserVHDL::~JUParserVHDL()
{
    JUMLog("dtor {%p}.", this);
}

// ========================================

JUSchemeTree* JUParserVHDL::parse()
{
    JUMLog("parse.");
    return NULL;
}