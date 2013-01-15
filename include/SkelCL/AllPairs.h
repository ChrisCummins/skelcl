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
/// \file AllPairs.h
///
///	\author Malte Friese <malte.friese@uni-muenster.de>
///

#ifndef ALLPAIRS_H
#define ALLPAIRS_H

#include <istream>
#include <string>

#include "detail/Skeleton.h"

namespace skelcl {

template <typename> class Matrix;
template <typename> class Reduce;
template <typename> class Zip;
template <typename> class Out;
namespace detail { class Program; }

template<typename> class AllPairs;

///
/// \class AllPairs
///
/// \brief An instance of the AllPairs skeleton describes a calculation of all two
///        pairs which can be performed on a device.
///
/// \tparam Tleft  Type of the left input data of the skeleton
///         Tright Type of the right input data of the skeleton
///         Tout   Type of the output data of the skeleton
///
/// On creation the AllPairs skeleton is customized with a given reduce and
/// zip skeleton.
/// The AllPairs skeleton can then be called by passing two matrix containers.
/// The AllPairs skeleton will compute the results of all two pairs of the row
/// and column-vectors from the matrices and store them in a result matrix.
/// More formally: When M and N are two matrices of dimension height x dimension
/// and dimension x width. Then D is the result matrix of dimension height x width.
/// Every D[i,j] is the scalar result from Reduce(Zip(M_i, N_j)), where M_i is
/// the ith row in M and N_j is the jth column in N.
///
template<typename Tleft,
         typename Tright,
         typename Tout>
class AllPairs<Tout(Tleft, Tright)> : public detail::Skeleton {

    public:
    // Konstruktoren
    AllPairs<Tout(Tleft, Tright)>(const Reduce<Tout(Tout)>& reduce, const Zip<Tout(Tleft, Tright)>& zip);

    AllPairs<Tout(Tleft, Tright)>(const std::string& source, const std::string& func = std::string("func"));

    // Ausführungsoperator
    template <typename... Args>
    Matrix<Tout> operator()(const Matrix<Tleft>& left,
                            const Matrix<Tright>& right,
                            Args&&... args);

    // Ausführungsoperator mit Referenz
    template <typename... Args>
    Matrix<Tout>& operator()(Out< Matrix<Tout> > output,
                             const Matrix<Tleft>& left,
                             const Matrix<Tright>& right,
                             Args&&... args);

    private:
    // Ausführen
    template <typename... Args>
    void execute(const detail::Program& program,
                 Matrix<Tout>& output,
                 const Matrix<Tleft>& left,
                 const Matrix<Tright>& right,
                 Args&&... args);

    // Programm erstellen
    detail::Program createAndBuildProgram() const;
    detail::Program createAndBuildProgramSpecial() const;
    detail::Program createAndBuildProgramGeneral() const;

    // Eingabe vorbereiten
    void prepareInput(const Matrix<Tleft>& left,
                      const Matrix<Tright>& right);

    // Ausgabe vorbereiten
    void prepareOutput(Matrix<Tout>& output,
                       const Matrix<Tleft>& left,
                       const Matrix<Tright>& right);

    // used by special implementation
    std::string _srcReduce;
    std::string _srcZip;
    std::string _funcReduce;
    std::string _funcZip;
    std::string _idReduce;

    // used by general implementation
    std::string _srcUser;
    std::string _funcUser;

    // used by both implementations
    bool _isSpecial;
    unsigned int _C;
    unsigned int _R;
    unsigned int _S;
};

} // namespace skelcl

#include "detail/AllPairsDef.h"

#endif // ALLPAIRS_H
