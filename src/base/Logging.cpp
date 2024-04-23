#include <cstdarg>
#include <cstdio>
#include <cstring>
// cstdio和stdio.h之间的主要区别在于它们如何将函数和类型引入到程序中。cstdio将所有的函数和类型引入到std命名空间，
// 而stdio.h则将它们引入到全局命名空间。因此，使用cstdio可以更好地遵循C++的命名空间规则，避免全局命名空间的污染。
#include <sys/time.h>
#include <ctime>

#include "base/Logging.h"
#include "base/AsyncLogging.h"

Logger::LogLevel Logger::mLogLevel = Logger::LogDebug;
std::string Logger::mLogFile = "/dev/stdout";
bool Logger::mIsStdout = true;

Logger::Logger() : mCurPtr(mData) {}

Logger::~Logger() {
  if (mIsStdout)
    printf("%s", mData);
  else
    AsyncLogging::instance()->append(mData, mCurPtr - mData);
}

void Logger::setLogFile(std::string file) {
  Logger::mLogFile = file;
  if (Logger::mLogFile == "/dev/stdout")
    Logger::mIsStdout = true;
  else
    Logger::mIsStdout = false;
}

std::string Logger::getLogFile() { return Logger::mLogFile; }

void Logger::setLogLevel(LogLevel level) { Logger::mLogLevel = level; }

Logger::LogLevel Logger::getLogLevel() { return Logger::mLogLevel; }

void Logger::write(Logger::LogLevel level, const char *file, const char *func,
                   int line, const char *format, ...) {
  // 非匹配日志等级的日志不被记录
  if (level > Logger::mLogLevel)
    return;

  struct timeval now = {0, 0};
  // 获取当前的系统时间，tz（timezone）为NULL表示使用本地时区
  gettimeofday(&now, nullptr);
  struct tm *sysTime = localtime(&(now.tv_sec));

  mThisLogLevel = level;

  // 记录时间
  sprintf(mCurPtr, "%d-%02d-%02d %02d:%02d:%02d", sysTime->tm_year + 1900,
          sysTime->tm_mon + 1, sysTime->tm_mday, sysTime->tm_hour,
          sysTime->tm_min, sysTime->tm_sec);
  mCurPtr += strlen(mCurPtr);

  // 记录日志等级
  if (level == Logger::LogDebug) {
      sprintf(mCurPtr, " <DEBUG> ");
  } else if (level == Logger::LogWarning) {
      sprintf(mCurPtr, " <WARNING> ");
  } else if (level == Logger::LogError) {
      sprintf(mCurPtr, " <ERROR> ");
  } else {
      return;
  }
  mCurPtr += strlen(mCurPtr);

  // 记录文件名，函数名和行号
  sprintf(mCurPtr, "%s:%s:%d ", file, func, line);
  mCurPtr += strlen(mCurPtr);

  // variable-argument list
  va_list valst;
  va_start(valst, format);

  vsnprintf(mCurPtr, sizeof(mData) - (mCurPtr - mData), format, valst);

  va_end(valst);

  mCurPtr += strlen(mCurPtr);
}