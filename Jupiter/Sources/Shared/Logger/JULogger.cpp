
#include "JULogger.h"
#include <QDebug>
#include <QTime>

void log(const char *format, ...)
{
    QString msg;

    va_list args;
    va_start(args, format);
    msg.vsprintf(format, args);
    va_end(args);
 
    log_internal(msg);
}

void mLog(const char *module, char *format, ...)
{
    QString qFormat = QString("[%1] %2").arg(module).arg(format);
    QString msg;

    va_list args;
    va_start(args, format);
    msg.vsprintf(Q(qFormat), args);
    va_end(args);

    log_internal(msg);
}

void log_internal(QString msg)
{
    QDateTime now;
    QString qFormat = QString("%1 %2").arg(QTime::currentTime().toString("hh:mm:ss")).arg(msg);
    qDebug(Q(qFormat));
}