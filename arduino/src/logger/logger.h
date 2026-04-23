#ifndef LOGGING_H
#define LOGGING_H

// To specify a log level, define LOG_LEVEL before including this file or set it here
// #define LOG_LEVEL 1
// Unfortunately, Arduino IDE does not support setting the LOG_LEVEL in the arduino.ino file

// Log Levels
#define LOG_LEVEL_DEBUG 0
#define LOG_LEVEL_INFO  1
#define LOG_LEVEL_WARN  2
#define LOG_LEVEL_ERROR 3

// Default log level
#ifndef LOG_LEVEL
#define LOG_LEVEL LOG_LEVEL_DEBUG
#endif


// INFO only prints [INFO]
#define LOG_PREFIX_INFO()        \
    Serial.print("[INFO] ");     

// Others print [LEVEL] file:line:
#define LOG_PREFIX_GENERIC(level) \
    Serial.print("["); Serial.print(level); Serial.print("] "); \
    Serial.print(__FILE__); Serial.print(":"); Serial.print(__LINE__); Serial.print(" ");

// Base log macros
// 1-argument INFO
#define _LOG_INFO_1(msg) \
  do { if (LOG_LEVEL <= LOG_LEVEL_INFO) { \
       LOG_PREFIX_INFO(); \
       Serial.println(msg); \
  }} while(0)

// 2-argument INFO
#define _LOG_INFO_2(msg, val) \
  do { if (LOG_LEVEL <= LOG_LEVEL_INFO) { \
       LOG_PREFIX_INFO(); \
       Serial.print(msg); Serial.print(" "); Serial.println(val); \
  }} while(0)


// DEBUG / WARN / ERROR behave the same: include file+line
#define _LOG_GENERIC_1(levelstr, level, msg) \
  do { if (LOG_LEVEL <= level) { \
       LOG_PREFIX_GENERIC(levelstr); \
       Serial.println(msg); \
  }} while(0)

#define _LOG_GENERIC_2(levelstr, level, msg, val) \
  do { if (LOG_LEVEL <= level) { \
       LOG_PREFIX_GENERIC(levelstr); \
       Serial.print(msg); Serial.print(" "); Serial.println(val); \
  }} while(0)


// Argument-counting selection helper
#define GET_LOG_MACRO(_1,_2,NAME,...) NAME


// Public macros

#define LOG_INFO(...) \
    GET_LOG_MACRO(__VA_ARGS__, _LOG_INFO_2, _LOG_INFO_1)(__VA_ARGS__)

#define LOG_DEBUG(...) \
    GET_LOG_MACRO(__VA_ARGS__, _LOG_GENERIC_2, _LOG_GENERIC_1)("DEBUG", LOG_LEVEL_DEBUG, __VA_ARGS__)

#define LOG_WARN(...) \
    GET_LOG_MACRO(__VA_ARGS__, _LOG_GENERIC_2, _LOG_GENERIC_1)("WARN", LOG_LEVEL_WARN, __VA_ARGS__)

#define LOG_ERROR(...) \
    GET_LOG_MACRO(__VA_ARGS__, _LOG_GENERIC_2, _LOG_GENERIC_1)("ERROR", LOG_LEVEL_ERROR, __VA_ARGS__)

#define RETURN_IF_NOT_DEBUG \
    do { if (LOG_LEVEL > LOG_LEVEL_DEBUG) return; } while(0)

#endif
