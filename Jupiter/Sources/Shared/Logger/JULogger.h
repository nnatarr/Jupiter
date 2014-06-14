
#ifndef __JULOGGER_H__
#define __JULOGGER_H__

#include <stdio.h>
#include <stdarg.h>
#include <QString>

extern inline void log(char *format, ...);
extern inline void mLog(char *module, char *format, ...);
extern inline void log_internal(QString msg);

#define JUMLog(format, ...) mLog(__JUModule__, format, ##__VA_ARGS__)
#define JULog(format, ...) log(format, ##__VA_ARGS__)

#endif