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
/// \file AllPairsDef.h
///
///	\author Malte Friese <malte.friese@uni-muenster.de>
///

#ifndef ALLPAIRS_DEF_H
#define ALLPAIRS_DEF_H

#include <algorithm>
#include <istream>
#include <iterator>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>

#define __CL_ENABLE_EXCEPTIONS
#include <CL/cl.h>
#undef __CL_ENABLE_EXCEPTIONS

#include <ssedit/TempSourceFile.h>
#include <ssedit/Function.h>

#include "../Distributions.h"
#include "../Matrix.h"
#include "../Reduce.h"
#include "../Zip.h"
#include "../Out.h"

#include "Assert.h"
#include "Device.h"
#include "KernelUtil.h"
#include "Logger.h"
#include "Program.h"
#include "Skeleton.h"
#include "Util.h"

namespace skelcl {

// Konstruktor
template<typename Tleft, typename Tright, typename Tout>
AllPairs<Tout(Tleft, Tright)>::AllPairs(const Reduce<Tout(Tout)>& reduce, const Zip<Tout(Tleft, Tright)>& zip)
    : detail::Skeleton(),
      _srcReduce(reduce.source()),
      _srcZip(zip.source()),
      _funcReduce(reduce.func()),
      _funcZip(zip.func()),
      _idReduce(reduce.id())
{
    LOG_DEBUG("Create new AllPairs object (", this, ")");
}

// Ausführungsoperator
template<typename Tleft, typename Tright, typename Tout>
template <typename... Args>
Matrix<Tout> AllPairs<Tout(Tleft, Tright)>::operator()(const Matrix<Tleft>& left,
                                                       const Matrix<Tright>& right,
                                                       Args&&... args)
{
    Matrix<Tout> output;
    this->operator()(out(output), left, right, std::forward<Args>(args)...);
    return output;
}

// Ausführungsoperator mit Referenz
template<typename Tleft, typename Tright, typename Tout>
template <typename... Args>
Matrix<Tout>& AllPairs<Tout(Tleft, Tright)>::operator()(Out< Matrix<Tout> > output,
                                                        const Matrix<Tleft>& left,
                                                        const Matrix<Tright>& right,
                                                        Args&&... args)
{
    ASSERT( (left.rowCount() > 0) && (right.columnCount() > 0) );
    ASSERT( left.columnCount() == right.rowCount() );
    ASSERT( left.columnCount() > 0 );

    detail::Program program = createAndBuildProgram();

    prepareInput(left, right);

    prepareOutput(output.container(), left, right);

    execute(program, output.container(), left, right, std::forward<Args>(args)...);

    updateModifiedStatus(output, std::forward<Args>(args)...);

    return output.container();
}

// Ausführen
template<typename Tleft, typename Tright, typename Tout>
template <typename... Args>
void AllPairs<Tout(Tleft, Tright)>::execute(const detail::Program& program,
                                            Matrix<Tout>& output,
                                            const Matrix<Tleft>& left,
                                            const Matrix<Tright>& right,
                                            Args&&... args)
{
    ASSERT( left.distribution().isValid() && right.distribution().isValid() );
    ASSERT( output.rowCount() == left.rowCount() && output.columnCount() == right.columnCount() );

    for (auto& devicePtr : left.distribution().devices()) { // ab hier neu
        auto& outputBuffer = output.deviceBuffer(*devicePtr);
        auto& leftBuffer   = left.deviceBuffer(*devicePtr);
        auto& rightBuffer  = right.deviceBuffer(*devicePtr);

        cl_uint elements[2]   = {output.rowCount(), output.columnCount()};
        cl_uint local[2]      = {32, 8}; // C, R
        cl_uint global[2]     = {detail::util::ceilToMultipleOf(elements[1], local[0]),
                                 detail::util::ceilToMultipleOf(elements[0], local[1]*4)/4}; // 4 SUBTILES
        cl_uint dimension     = {left.columnCount()};

        try {
            cl::Kernel kernel(program.kernel(*devicePtr, "SCL_ALLPAIRS"));

            kernel.setArg(0, leftBuffer.clBuffer());
            kernel.setArg(1, rightBuffer.clBuffer());
            kernel.setArg(2, outputBuffer.clBuffer());
            kernel.setArg(3, dimension);   // dimension
            kernel.setArg(4, elements[0]); // height
            kernel.setArg(5, elements[1]); // width

            LOG_DEBUG("dim: ", dimension, " height: ", elements[0], " width: ",elements[1]);
            LOG_DEBUG("local: ", local[0],",", local[1], " global: ", global[0],",",global[1]);

            detail::kernelUtil::setKernelArgs(kernel, *devicePtr, 6,
                                              std::forward<Args>(args)...);


            // keep buffers and arguments alive / mark them as in use
            auto keepAlive = detail::kernelUtil::keepAlive(*devicePtr,
                                                           leftBuffer.clBuffer(),
                                                           rightBuffer.clBuffer(),
                                                           outputBuffer.clBuffer(),
                                                           std::forward<Args>(args)...);

            // after finishing the kernel invoke this function ...
            auto invokeAfter =  [=] () { (void)keepAlive; };

            devicePtr->enqueue(kernel, cl::NDRange(global[0], global[1]), cl::NDRange(local[0], local[1]),
                               cl::NullRange, // offset
                               invokeAfter);

        } catch (cl::Error& err) {
            ABORT_WITH_ERROR(err);
        }
    }
    LOG_INFO("AllPairs kernel started");
}

// Programm erstellen
template<typename Tleft, typename Tright, typename Tout>
detail::Program AllPairs<Tout(Tleft, Tright)>::createAndBuildProgram() const
{
    ASSERT_MESSAGE( !_srcReduce.empty(),
                    "Tried to create program with empty user reduce source." );
    ASSERT_MESSAGE( !_srcZip.empty(),
                    "Tried to create program with empty user zip source." );

    // _srcReduce: replace func by USR_REDUCE
    ssedit::TempSourceFile reduceTemp(_srcReduce);

    auto func = reduceTemp.findFunction(_funcReduce); ASSERT(func.isValid());
    reduceTemp.commitRename(func, "TMP_REDUCE");
    reduceTemp.writeCommittedChanges();

    std::ifstream rFile(reduceTemp.getFileName());
    std::string rSource( (std::istreambuf_iterator<char>(rFile)),
                         std::istreambuf_iterator<char>() );

    // _srcZip: replace func by USR_ZIP
    ssedit::TempSourceFile zipTemp(_srcZip);

    func = zipTemp.findFunction(_funcReduce); ASSERT(func.isValid());
    zipTemp.commitRename(func, "TMP_ZIP");
    zipTemp.writeCommittedChanges();

    std::ifstream zFile(zipTemp.getFileName());
    std::string zSource( (std::istreambuf_iterator<char>(zFile)),
                         std::istreambuf_iterator<char>() );

    // create program
    std::string s(Matrix<Tout>::deviceFunctions());

    // identity
    s.append("#define SCL_IDENTITY ").append(_idReduce);

    s.append("\n");

    // reduce user source
    s.append(rSource);

    s.append("\n");

    // zip user source
    s.append(zSource);

    // allpairs skeleton source
    s.append(
      #include "AllPairsKernel.cl"
    );

    //LOG_DEBUG(s);

    auto program = detail::Program(s, detail::util::hash("//AllPairs\n"
                                                         + Matrix<Tout>::deviceFunctions()
                                                         + _idReduce
                                                         + rSource
                                                         + zSource));
    // modify program
    //if (!program.loadBinary()) {
    program.transferParameters("TMP_REDUCE", 2, "SCL_ALLPAIRS"); // problem: reduce parameter a und zip parameter a
    program.transferArguments("TMP_REDUCE", 2, "USR_REDUCE");
    program.transferParameters("TMP_ZIP", 2, "SCL_ALLPAIRS"); // reihenfolge? erst args von reduce dann zip?
    program.transferArguments("TMP_ZIP", 2, "USR_ZIP");

    program.renameFunction("TMP_REDUCE", "USR_REDUCE");
    program.renameFunction("TMP_ZIP", "USR_ZIP");

    program.adjustTypes<Tleft, Tright, Tout>();
    //}

    program.build();

    return program;
}

// Eingabe vorbereiten
template<typename Tleft, typename Tright, typename Tout>
void AllPairs<Tout(Tleft, Tright)>::prepareInput(const Matrix<Tleft>& left,
                                                 const Matrix<Tright>& right)
{
    // set distributions
    left.setDistribution(detail::BlockDistribution< Matrix<Tleft> >()); // some rows of matrix
    right.setDistribution(detail::CopyDistribution< Matrix<Tright> >()); // whole matrix

    // create buffers if required
    left.createDeviceBuffers();
    right.createDeviceBuffers();

    // copy data to devices
    left.startUpload();
    right.startUpload();
}

// Ausgabe vorbereiten
template<typename Tleft, typename Tright, typename Tout>
void AllPairs<Tout(Tleft, Tright)>::prepareOutput(Matrix<Tout>& output,
                                                  const Matrix<Tleft>& left,
                                                  const Matrix<Tright>& right)
{
    // set size
    if (output.rowCount() != left.rowCount() || output.columnCount() != right.columnCount())
        output.resize(typename Matrix<Tout>::size_type(left.rowCount(), right.columnCount()));

    // set distribution
    output.setDistribution(detail::BlockDistribution< Matrix<Tout> >());

    //create buffers if required
    output.createDeviceBuffers();
}

} // namespace skelcl

#endif // ALLPAIRS_DEF_H
