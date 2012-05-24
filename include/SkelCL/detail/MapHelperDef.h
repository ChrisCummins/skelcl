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
/// \file MapHelperDef.h
///
/// \author Michel Steuwer <michel.steuwer@uni-muenster.de>
///

#ifndef MAP_HELPER_DEF_H_
#define MAP_HELPER_DEF_H_

#include "../Distribution.h"

#include "Container.h"
#include "Logger.h"

namespace skelcl {

namespace detail {

template<typename Tin, typename Tout>
MapHelper<Tout(Tin)>::MapHelper(const Program& program,
                                const std::string& funcName)
  : _program(program)
{
  prepareAndBuildProgram(funcName);
  LOG_DEBUG("Create new Map object (", this, ")");
}

template<typename Tin, typename Tout>
void MapHelper<Tout(Tin)>::prepareAndBuildProgram(const std::string& funcName)
{
  if (!_program.loadBinary()) {
    _program.transferParameters(funcName, 1, "SCL_MAP");
    _program.transferArguments(funcName, 1, "SCL_FUNC");
    _program.renameFunction(funcName, "SCL_FUNC");
    _program.adjustTypes<Tin, Tout>();
  }
  _program.build();
}

template <typename Tin, typename Tout>
void MapHelper<Tout(Tin)>::prepareInput(const detail::Container<Tin>& input)
{
  // set default distribution if required
  if (input.distribution() == nullptr) {
    input.setDistribution(Distribution::Block());
  }
  // create buffers if required
  input.createDeviceBuffers();
  // copy data to devices
  input.startUpload();
}

template <typename Tin, typename Tout>
void MapHelper<Tout(Tin)>::prepareOutput(detail::Container<Tout>& output,
                                         const detail::Container<Tin>& input)
{
  if (static_cast<void*>(&output) == static_cast<const void*>(&input)) {
    return; // already prepared in prepareInput
  }
  // resize container if required
  if (output.size() < input.size()) {
    output.resize(input.size());
  }
  // adopt distribution from input
  output.setDistribution(input.distribution());
  // create buffers if required
  output.createDeviceBuffers();
}

} // namespace detail

} // namespace skelcl

#endif // MAP_HELPER_DEF_H_
