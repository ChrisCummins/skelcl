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
/// \file Device.h
///
/// \author Michel Steuwer <michel.steuwer@uni-muenster.de>
///

#ifndef DEVICE_H_
#define DEVICE_H_

#include <algorithm>
#include <functional>
#include <string>

#define __CL_ENABLE_EXCEPTIONS
#include <CL/cl.hpp>
#undef  __CL_ENABLE_EXCEPTIONS

namespace skelcl {

namespace detail {

class DeviceBuffer;

///
/// \class Device
///
/// \brief An instance of the Device class represents an OpenCL device.
///
/// This class encapsulates functionality like starting data transfers, kernel
/// executions, querying information about the device, etc.
///
class Device {
public:
  typedef size_t id_type;

  ///
  /// \brief No default constuction allowed
  ///
  Device() = delete;

  ///
  /// \brief Constructs a new Device object encapsulating the given OpenCL
  ///        device
  ///
  /// \param device   The OpenCL device to encapsulate
  ///        platform The OpenCL platform for the device
  ///        id       A globally unique identifier in the range of
  ///                 [0, number of devices)
  ///
  Device(const cl::Device& device,
         const cl::Platform& platform,
         const id_type id);

  ///
  /// \brief Default copy constructor
  ///
  Device(const Device&) = default;

  ///
  /// \brief Default assignment operator
  ///
  Device& operator=(const Device&) = default;

  ///
  /// \brief Default destructor
  ///
  ~Device() = default;

  ///
  /// \brief Enqueues the execution of an OpenCL kernel object on the device
  ///
  /// \param kernel The OpenCL kernel to be enqueued
  ///        global The total number of OpenCL Work Items to be used in the
  ///               kernel execution
  ///        local  The number of OpenCL Work Items to form an OpenCL Work Group
  ///        offset An Offset to the global IDs of the OpenCL Work Items
  ///
  /// \return An OpenCL Event object which can be used to wait for the
  ///         operation to complete
  ///
  cl::Event enqueue(const cl::Kernel& kernel,
                    const cl::NDRange& global,
                    const cl::NDRange& local,
                    const cl::NDRange& offset = cl::NullRange,
                    const std::function<void()> callback = nullptr) const;

  ///
  /// \brief Enqueues a memory operation to copy data to the devices memory
  ///
  /// \param buffer     The Buffer on the device to which the data should be
  ///                   copied
  ///        iterator   Iterator pointing to the data which should be copied
  ///                   to the device
  ///        hostOffset Number of elements to be skipped at the start of
  ///                   iterator
  ///
  /// \return An OpenCL Event object which can be used to wait for the
  ///         operation to complete
  ///
  template <typename RandomAccessIterator>
  cl::Event enqueueWrite(const DeviceBuffer& buffer,
                         RandomAccessIterator iterator,
                         size_t hostOffset = 0) const;

  ///
  /// \brief Enqueues a memory operation to copy data to the devices memory
  ///
  /// \param buffer       The Buffer on the device to which the data should be
  ///                     copied
  ///        hostPointer  Pointer pointing to the data which should be copied
  ///                     to the device
  ///        hostOffset   Number of elements to be skipped at the start of
  ///                     hostPointer
  ///
  /// \return An OpenCL Event object which can be used to wait for the
  ///         operation to complete
  ///
  cl::Event enqueueWrite(const  DeviceBuffer& buffer,
                         void*  hostPointer,
                         size_t hostOffset = 0) const;
  ///
  /// \brief Enqueues a memory operation to copy data from the devices memory
  ///
  /// \param buffer     The Buffer on the device from which the data should be
  ///                   copied
  ///        iterator   Iterator pointing to the memory location to which
  ///                   the data should be copied
  ///        hostOffset Number of elements to be skipped at the start of
  ///                   iterator
  ///
  /// \return An OpenCL Event object which can be used to wait for the
  ///         operation to complete
  ///
  template <typename RandomAccessIterator>
  cl::Event enqueueRead(const DeviceBuffer& buffer,
                        RandomAccessIterator iterator,
                        size_t hostOffset = 0) const;

  ///
  /// \brief Enqueues a memory operation to copy data from the devices memory
  ///
  /// \param buffer       The Buffer on the device from which the data should be
  ///                     copied
  ///        hostPointer  Pointer pointing to the memory location to which
  ///                     the data should be copied
  ///        hostOffset   Number of elements to be skipped at the start of
  ///                     hostPointer
  ///
  /// \return An OpenCL Event object which can be used to wait for the
  ///         operation to complete
  ///
  cl::Event enqueueRead(const DeviceBuffer& buffer,
                        void* const hostPointer,
                        size_t hostOffset = 0) const;

  ///
  /// \brief Enqueues a memory operation to copy data from one buffer to the other.
  ///        Both buffers should reside on the same device (or at least in the same
  ///        context).
  ///
  /// \param from       The Buffer from which the data is copied
  ///        to         The Buffer where the data is copied into. This buffer has
  ///                   to be at least the same size as the from buffer.
  ///        fromOffset Offset used inside the from buffer. The value has to be
  ///                   given in Bytes! In total the size of the from buffer minus
  ///                   the from offset bytes are copied to the to buffer.
  ///        toOffset   Offset used inside the to buffer. The value has to be given
  ///                   in Bytes!
  ///
  /// \return An OpenCL Event object which can be used to wait for the
  ///         operation to complete
  ///
  cl::Event enqueueCopy(const DeviceBuffer& from,
                        const DeviceBuffer& to,
                        size_t fromOffset = 0,
                        size_t toOffset = 0) const;

  ///
  /// \brief Wait for all operations enqueued to finish
  ///
  void wait() const;

  ///
  /// \brief Returns the globally uniqueue identifier
  ///
  /// \return The globally unique identifier identifying this specific device
  ///
  id_type id() const;

  ///
  /// \brief Returns the name of the device as a string
  ///
  /// \return The name of the device as a string
  ///
  std::string name() const;

  ///
  /// \brief Returns the name of the vendor of the device as a string
  ///
  /// \return The name of the vendor of the device as a string
  ///
  std::string vendorName() const;

  ///
  /// \brief Returns the maximal clock frequency of the device
  ///
  /// \return The maximal clock frequency of the device
  ///
  unsigned int maxClockFrequency() const;

  ///
  /// \brief Returns the maximal number of compute units of the device
  ///
  /// \return The maximal number of compute units of the device
  ///
  unsigned int maxComputeUnits() const;

  ///
  /// \brief Returns the maximal number of OpenCL Work Items in one OpenCL Work
  ///        Group for the device
  ///
  /// \return The maximal number of OpenCL Work Items in one OpenCL Work Group
  ///
  size_t maxWorkGroupSize() const;

  ///
  /// \brief Returns the maximal numbers of OpenCL Work Groups for the device
  ///
  /// \return The maximal number of OpenCL Work Groups
  ///
  size_t maxWorkGroups() const;

  ///
  /// \brief Returns the maximal global memory size for the device
  ///
  /// \return The maximal global memory size for the device
  ///
  unsigned long globalMemSize() const;

  ///
  /// \brief Returns the maximal local memory size for the device
  ///
  /// \return The maximal local memory size for the device
  ///
  unsigned long localMemSize() const;

  ///
  /// \brief Get access to the OpenCL Context for the device
  ///
  /// \return A reference to the OpenCL Context associated with the device
  ///
  const cl::Context& clContext() const;

  ///
  /// \brief Get access to the OpenCL Device object
  ///
  /// \return A reference to the OpenCL Device object
  ///
  const cl::Device& clDevice() const;

private:
  cl::Device        _device;
  cl::Context       _context;
  cl::CommandQueue  _commandQueue;
  id_type           _id;
};

template <typename RandomAccessIterator>
cl::Event Device::enqueueWrite(const DeviceBuffer& buffer,
                          RandomAccessIterator iterator,
                          size_t hostOffset) const {
  return enqueueWrite(buffer,
                      static_cast<void*>(&(*iterator)),
                      hostOffset);
}

template <typename RandomAccessIterator>
cl::Event Device::enqueueRead(const DeviceBuffer& buffer,
                         RandomAccessIterator iterator,
                         size_t hostOffset) const {
  return enqueueRead(buffer,
                     static_cast<void*>(&(*iterator)),
                     hostOffset);
}
#if 0
// unclear where used and why defined as a free function, same functionality in
// DeviceList::barrier
// free helper functions
template <typename DeviceIterator>
void barrier(DeviceIterator begin, DeviceIterator end) {
  std::for_each( begin, end,
                 std::mem_fn(&skelcl::detail::Device::wait) );
}
#endif
} // namespace detail

} // namespace skelcl

#endif // DEVICE_H_
