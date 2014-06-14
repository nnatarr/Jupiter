
#ifndef __JULOGGER_H__
#define __JULOGGER_H__

#include <stdio.h>
#include <stdarg.h>
#include <QString>

extern inline void log(const char *format, ...);
extern inline void mLog(const char *module, char *format, ...);
extern inline void log_internal(QString msg);

#define JUMLog(format, ...) mLog(__JUMODULE__, format, ##__VA_ARGS__)
#define JULog(format, ...) log(format, ##__VA_ARGS__)

#endif