
#ifndef __JUPARSERVHDL_H__
#define __JUPARSERVHDL_H__

#include "../JUParser.h"

class JUParserVHDL : public JUParser
{
public:
    JUParserVHDL();
    virtual ~JUParserVHDL();

    virtual JUSchemeTree* parse();
};

#endif