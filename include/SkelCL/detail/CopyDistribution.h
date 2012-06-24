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
/// \file CopyDistribution.h
///
///	\author Michel Steuwer <michel.steuwer@uni-muenster.de>
///

#ifndef COPY_DISTRIBUTION_H_
#define COPY_DISTRIBUTION_H_

#include "Distribution.h"

namespace skelcl {

namespace detail {

template <typename> class CopyDistribution;

template <template <typename> class C, typename T>
class CopyDistribution< C<T> > : public Distribution< C<T> > {
public:
  CopyDistribution(const DeviceList& deviceList = detail::globalDeviceList,
                   std::function<T(const T&, const T&)> combineFunc = nullptr);

  template <typename U>
  CopyDistribution( const CopyDistribution< C<U> >& rhs);

  ~CopyDistribution();

  bool isValid() const;

  void startUpload(C<T>& container,
                   Event* events) const;

  void startDownload(C<T>& container,
                     Event* events) const;

  size_t sizeForDevice(const Device::id_type deviceID,
                       const size_t totalSize) const;

  bool dataExchangeOnDistributionChange(Distribution< C<T> >& newDistribution);

  std::function<T(const T&, const T&)> combineFunc() const;

protected:
  bool doCompare(const Distribution< C<T> >& rhs) const;

private:
  std::function<T(const T&, const T&)> _combineFunc;
};

} // namespace detail

} // namespace skelcl

#include "CopyDistributionDef.h"

#endif // COPY_DISTRIBUTION_H_
