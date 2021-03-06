/*****************************************************************************
 * Copyright (c) 2011-2013 The SkelCL Team as listed in CREDITS.txt          *
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
/// \file Timer.cpp
///
/// \author Sebastian Albers <s.albers@uni-muenster.de>
///

#include <chrono>

#include "pvsutil/Timer.h"

namespace pvsutil {
  
Timer::Timer()
  : _startTime(std::chrono::high_resolution_clock::now())
{
}

void Timer::restart()
{
  _startTime = std::chrono::high_resolution_clock::now();
}
  
Timer::time_type Timer::stop() {
  auto sinceStart =   std::chrono::high_resolution_clock::now() - _startTime;
  using std::chrono::milliseconds; // avoid too long next line
  return std::chrono::duration_cast<milliseconds>(sinceStart).count();
}
  
} // namespace pvsutil
