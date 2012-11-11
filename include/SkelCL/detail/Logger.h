/*****************************************************************************
 * Copyright (c) 2011-2012 The SkelCL Team as listed in CREDITS.txt          *
 * http://skelcl.uni-muenster.de                                             *
 *                                                                           *
 * This file is part of SkelCL.                                              *
 * SkelCL is available under multiple licenses.                              *
 * The different licenses are subject to terms and condition as provided     *
 * in the files specifying the license. See "LICENSE.txt" for details        *
 *                                                                           *
 *****************************************************************************
 *                                                                           *
 * SkelCL is free software: you can redistribute it and/or modify            *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation, either version 3 of the License, or         *
 * (at your option) any later version. See "LICENSE-gpl.txt" for details.    *
 *                                                                           *
 * SkelCL is distributed in the hope that it will be useful,                 *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the              *
 * GNU General Public License for more details.                              *
 *                                                                           *
 *****************************************************************************
 *                                                                           *
 * For non-commercial academic use see the license specified in the file     *
 * "LICENSE-academic.txt".                                                   *
 *                                                                           *
 *****************************************************************************
 *                                                                           *
 * If you are interested in other licensing models, including a commercial-  *
 * license, please contact the author at michel.steuwer@uni-muenster.de      *
 *                                                                           *
 *****************************************************************************/
 
///
/// \file Logger.h
///
/// \author Michel Steuwer <michel.steuwer@uni-muenster.de>
///

#ifndef LOGGER_H_
#define LOGGER_H_

#include <ostream>

#define __CL_ENABLE_EXCEPTIONS
#ifdef __APPLE__
#  include "../../CL/cl.hpp"
#else
#  include <CL/cl.hpp>
#endif
#undef  __CL_ENABLE_EXCEPTIONS

namespace skelcl {

namespace detail {

class Logger {
public:
  struct Severity {
    enum Type {
      Error = 0,
      Warning,
      Info,
      Debug,
      DebugInfo
    };
  };

  Logger();

  Logger(std::ostream& output, Severity::Type severity);

  void setOutput(std::ostream& output);

  void setLoggingLevel(Severity::Type severity);

  template <typename... Args>
  void log(Severity::Type severity, const char* file, int line,
           Args&&... args);

private:
  void logArgs(std::ostream& output);

  template <typename... Args>
  void logArgs(std::ostream& output, const cl::Error& err, Args&&... args);

  template <typename T, typename... Args>
  void logArgs(std::ostream& output, T value, Args&&... args);

  Severity::Type  _severity;
  std::ostream*   _output;
};

#define LOG(severity, ...)\
  skelcl::detail::defaultLogger.log(severity, __FILE__, __LINE__,\
                                    __VA_ARGS__)

#define LOG_ERROR(...)\
  LOG(skelcl::detail::Logger::Severity::Error, __VA_ARGS__)

#define ABORT_WITH_ERROR(err)\
  LOG_ERROR(err); abort()

#ifdef NDEBUG

#define LOG_WARNING(...)    (void(0))
#define LOG_INFO(...)       (void(0))
#define LOG_DEBUG(...)      (void(0))
#define LOG_DEBUG_INFO(...) (void(0))

#else  // DEBUG

#define LOG_WARNING(...)\
  LOG(skelcl::detail::Logger::Severity::Warning, __VA_ARGS__)

#define LOG_INFO(...)\
  LOG(skelcl::detail::Logger::Severity::Info, __VA_ARGS__)

#define LOG_DEBUG(...)\
  LOG(skelcl::detail::Logger::Severity::Debug, __VA_ARGS__)

#define LOG_DEBUG_INFO(...)\
  LOG(skelcl::detail::Logger::Severity::DebugInfo, __VA_ARGS__)

#endif // NDEBUG

// Default logger connected per default to std::clog
extern Logger defaultLogger;

} // namespace detail

} // namespace skelcl

#include "LoggerDef.h"

#endif // LOGGER_H_

