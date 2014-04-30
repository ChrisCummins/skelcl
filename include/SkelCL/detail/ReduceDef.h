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
/// \file ReduceDef.h
///
/// \author Michel Steuwer <michel.steuwer@uni-muenster.de>
///

#ifndef REDUCE_DEF_H_
#define REDUCE_DEF_H_

#include <algorithm>
#include <array>
#include <fstream>
#include <functional>
#include <iostream>
#include <istream>
#include <iterator>
#include <numeric>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#define __CL_ENABLE_EXCEPTIONS
#include <CL/cl.h>
#undef  __CL_ENABLE_EXCEPTIONS

#include <pvsutil/Assert.h>
#include <pvsutil/Logger.h>

#include "../Distributions.h"
#include "../Out.h"
#include "../Source.h"

#include "Device.h"
#include "DeviceBuffer.h"
#include "DeviceList.h"
#include "KernelUtil.h"
#include "Program.h"
#include "Skeleton.h"
#include "Util.h"


namespace skelcl {

template <typename T>
Reduce<T(T)>::Reduce(const Source& source,
                     const std::string& id,
                     const std::string& funcName)
    : detail::Skeleton(), _id(id), _funcName(funcName), _userSource(source), _program{}
{
}


template <typename T>
template <typename... Args>
Vector<T> Reduce<T(T)>::operator()(const Vector<T>& input,
                                   Args&&... args)
{
  Vector<T> output;
  this->operator()(out(output), input, std::forward<Args>(args)...);
  return output;
}



template <typename T>
template <typename... Args>
Vector<T>& Reduce<T(T)>::operator()(Out<Vector<T>> output,
                                    const Vector<T>& input,
                                    Args&&... args)
{
  size_t global_size = std::min( static_cast<size_t>(8192), input.size() );

  prepareInput(input);

  Vector<T> tmpOutput;
  prepareOutput( tmpOutput,          input,     global_size );
  prepareOutput( output.container(), tmpOutput, 1           );


  ASSERT(input.distribution().devices().size() == 1);
  _program = createPrepareAndBuildProgram();

  // TODO: relax to multiple devices later
  auto&  device  = *(input.distribution().devices().front());



  execute_first_step( device,
                      input.deviceBuffer(device),
                      tmpOutput.deviceBuffer(device),
                      input.size(),
                      global_size,
                      args... );


//  tmpOutput.dataOnDeviceModified();

//for( int i = 0; i < tmpOutput.size(); ++i)
//  std::cout << "tmpOutput[" << i << "]: " << tmpOutput[i] << std::endl;

  output.container()[0]=123;
  execute_second_step( device,
                       tmpOutput.deviceBuffer(device),
                       output.container().deviceBuffer(device),
                       global_size,
                       args... );




  // ... finally update modification status.
  updateModifiedStatus(output, std::forward<Args>(args)...);

  return output.container();
}


// private member functions

template <typename T>
void Reduce<T(T)>::prepareInput(const Vector<T>& input)
{
  // set default distribution if required
  if (!input.distribution().isValid()) {
    input.setDistribution(detail::SingleDistribution<Vector<T>>());
  }
  // create buffers if required
  input.createDeviceBuffers();
  // copy data to devices
  input.startUpload();
}


template <typename T>
void Reduce<T(T)>::prepareOutput(Vector<T>& output,
                                 const Vector<T>& input,
                                 const size_t size)
{
  ASSERT(size > 0);
  if (static_cast<const void*>(&output) == static_cast<const void*>(&input)) {
    return; // already prepared in prepareInput
  }
  // resize container if required
  if (output.size() < size) {
    output.resize(size);
  }
  // set output distribution
  output.setDistribution(input.distribution());
  // create buffers if required
  output.createDeviceBuffers();
}


template <typename T>
template <typename... Args>
void Reduce<T(T)>::execute_first_step(const detail::Device& device,
                                      const detail::DeviceBuffer& input,
                                      detail::DeviceBuffer& output,
                                      size_t data_size,
                                      size_t global_size,
                                      Args&&... args)
{
//  ASSERT( level->inputPtr->distribution().isValid() );

  try {
    cl::Kernel kernel = _program->kernel(device, "SCL_REDUCE_1");

    const cl::Device dev{device.clDevice()};

    kernel.setArg(0, input.clBuffer());
    kernel.setArg(1, output.clBuffer());
    kernel.setArg(2, static_cast<unsigned int>(data_size));
    kernel.setArg(3, static_cast<unsigned int>(global_size));

   // detail::kernelUtil::setKernelArgs(kernel, device, 4,
   //                                   std::forward<Args>(args)...);

    auto keepAlive = detail::kernelUtil::keepAlive(device,
                                                   input.clBuffer(),
                                                   output.clBuffer(),
                                                   std::forward<Args>(args)...);

    // after finishing the kernel invoke this function ...
    auto invokeAfter =  [=] () {
                                  (void)keepAlive;
                               };

    device.enqueue(kernel,
                   cl::NDRange(global_size), cl::NDRange(global_size)/*ANPASSEN!!!*/,
                   cl::NullRange, // offset
                   invokeAfter);
  } catch (cl::Error& err) {
    ABORT_WITH_ERROR(err);
  }
}


template <typename T>
template <typename... Args>
void Reduce<T(T)>::execute_second_step(const detail::Device& device,
                                       const detail::DeviceBuffer& input,
                                             detail::DeviceBuffer& output,
                                             size_t data_size,
                                             Args&&... args)
{
  //ASSERT( level->inputPtr->distribution().isValid() );
  //ASSERT: global_size < MAX_LOCAL_SIZE

    try {
    cl::Kernel kernel = _program->kernel(device, "SCL_REDUCE_2");
    const cl::Device dev{device.clDevice()};

    const size_t max_local_size = kernel.getWorkGroupInfo<CL_KERNEL_WORK_GROUP_SIZE>(dev);
    size_t global_size = std::min( data_size, max_local_size);

    kernel.setArg(0, input.clBuffer() );
    kernel.setArg(1, output.clBuffer() );
    kernel.setArg(2, cl::__local(global_size * sizeof(T) ) );
    kernel.setArg(3, static_cast<unsigned int>(data_size) );
    kernel.setArg(4, static_cast<unsigned int>(global_size) );

   // detail::kernelUtil::setKernelArgs(kernel, device, 4,
   //                                   std::forward<Args>(args)...);


    auto keepAlive = detail::kernelUtil::keepAlive(device,
                                                   input.clBuffer(),
                                                   output.clBuffer(),
                                                   std::forward<Args>(args)...);

    // after finishing the kernel invoke this function ...
    auto invokeAfter =  [=] () {
                                  (void)keepAlive;
                               };

//    void* buffer = static_cast<void*>(malloc(1 * sizeof(float)));
//    detail::Event event{device.enqueueRead(input, buffer)};
//    event.wait();

//    for( int i=0; i < 100; ++i)
//    std::cout << "INPUT[" << i << "]: " << static_cast<float*>(buffer)[i] << std::endl;



    device.enqueue(kernel,
                   cl::NDRange(global_size), cl::NDRange(global_size),
                   cl::NullRange, // offset
                   invokeAfter);


  } catch (cl::Error& err) {
    ABORT_WITH_ERROR(err);
  }
}


template <typename T>
std::shared_ptr<skelcl::detail::Program>
  Reduce<T(T)>::createPrepareAndBuildProgram()
{
  ASSERT_MESSAGE(!_userSource.empty(),
    "Tried to create program with empty user source.");
  // first: device specific functions
  std::string s(detail::CommonDefinitions::getSource());
  // second: user defined source
  s.append(_userSource);
  // last: append skeleton implementation source
  s.append(
    #include "ReduceKernel.cl"
  );

  auto program = std::make_shared<detail::Program>(s
        /*, skelcl::detail::util::hash("//Reduce\n"+source)*/);
  //if (!program->loadBinary()) {
    program->transferParameters(_funcName, 2, "SCL_REDUCE");
    program->transferArguments(_funcName, 2, "SCL_FUNC");
    program->renameFunction(_funcName, "SCL_FUNC");
    program->adjustTypes<T>();
  //}
  program->build();
  return program;
}



} // namespace skelcl

#endif // REDUCE_DEF_H_
