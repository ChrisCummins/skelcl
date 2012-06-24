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
/// SingleDistributionDef.h
///
/// \author Michel Steuwer <michel.steuwer@uni-muenster.de>
///

#ifndef SINGLE_DISTRIBUTION_DEF_H_
#define SINGLE_DISTRIBUTION_DEF_H_

#include <memory>

#include "Assert.h"
#include "DeviceList.h"

namespace skelcl {

namespace detail {

template <template <typename> class C, typename T>
SingleDistribution< C<T> >::SingleDistribution(std::shared_ptr<Device> device)
  : Distribution< C<T> >( {device} )
{
}

template <template <typename> class C, typename T>
template <typename U>
SingleDistribution< C<T> >::SingleDistribution( const SingleDistribution< C<U> >& rhs)
  : Distribution< C<T> >( rhs )
{
}

template <template <typename> class C, typename T>
SingleDistribution< C<T> >::~SingleDistribution()
{
}

template <template <typename> class C, typename T>
bool SingleDistribution< C<T> >::isValid() const
{
  return true;
}

template <template <typename> class C, typename T>
void SingleDistribution< C<T> >::startUpload(C<T>& container,
                                             Event* events) const
{
  ASSERT(events != nullptr);

  auto& buffer = container.deviceBuffer(*(this->_devices.front()));

  auto event = this->_devices.front()->enqueueWrite(buffer,
                                                    container.hostBuffer().begin());

  events->insert(event);
}

template <template <typename> class C, typename T>
void SingleDistribution< C<T> >::startDownload(C<T>& container,
                                               Event* events) const
{
  ASSERT(events != nullptr);

  auto& buffer = container.deviceBuffer(*(this->_devices.front()));

  auto event = this->_devices.front()->enqueueRead(buffer,
                                                   container.hostBuffer().begin());

  events->insert(event);
}

template <template <typename> class C, typename T>
size_t SingleDistribution< C<T> >::sizeForDevice(const Device::id_type /*id*/,
                                                 const size_t totalSize) const
{
  return totalSize;
}

template <template <typename> class C, typename T>
bool SingleDistribution< C<T> >::dataExchangeOnDistributionChange(
                                   Distribution< C<T> >& newDistribution)
{
  auto single = dynamic_cast<SingleDistribution< C<T> >*>(&newDistribution);

  if (single == nullptr) { // distributions differ => data exchange
    return true;
  } else { // new distribution == single distribution
    if (this->_devices.front() == single->_devices.front()) {
      return false; // both distributions have same device => no data exchange
    } else {
      return true;  // different devices => data exchange
    }
  }
}

template <template <typename> class C, typename T>
bool SingleDistribution< C<T> >::doCompare(const Distribution< C<T> >& rhs) const
{
  bool ret = false;
  auto const copyRhs = dynamic_cast<const SingleDistribution< C<T> >*>(&rhs);
  if (copyRhs) {
    ret = true;
  }
  return ret;
}

} // namespace detail

} // namespace skelcl

#endif // SINGLE_DISTRIBUTION_DEF_H_
