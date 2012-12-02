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
/// \author Malte Friese <malte.friese@uni-muenster.de>
///

#include <gtest/gtest.h>

#include <cstdio>
#include <vector>

#include <SkelCL/SkelCL.h>
#include <SkelCL/Vector.h>
#include <SkelCL/AllPairs.h>
#include <SkelCL/Zip.h>
#include <SkelCL/Reduce.h>
#include <SkelCL/detail/Logger.h>

class AllPairsTest : public ::testing::Test {
protected:
    AllPairsTest() {
        //skelcl::detail::defaultLogger.setLoggingLevel(skelcl::detail::Logger::Severity::Debug);
        skelcl::init(skelcl::nDevices(1));
    }

    ~AllPairsTest() {
        skelcl::terminate();
    }
};

// Tests Constructor
TEST_F(AllPairsTest, CreateAllPairsWithZipAndReduce) {
    skelcl::Zip<float(float, float)> zip("float func(float x, float y){ return x*y; }");
    skelcl::Reduce<float(float)> reduce("float func(float x, float y){ return x+y; }");
    skelcl::AllPairs<float(float, float)> allpairs(reduce, zip);
}

// Tests kernel with matrixmultiplication of two 64x64 matrices
TEST_F(AllPairsTest, SquareMatrices64x64AllPairs) {
    skelcl::Zip<float(float, float)> zip("float func(float x, float y){ return x*y; }");
    skelcl::Reduce<float(float)> reduce("float func(float x, float y){ return x+y; }");
    skelcl::AllPairs<float(float, float)> allpairs(reduce, zip);

    std::vector<float> tmpleft(4096);
    for (size_t i = 0; i < tmpleft.size(); ++i)
      tmpleft[i] = i;
    EXPECT_EQ(4096, tmpleft.size());

    std::vector<float> tmpright(tmpleft);
    EXPECT_EQ(4096, tmpright.size());

    skelcl::Matrix<float> left(tmpleft, 64);
    EXPECT_EQ(64, left.rowCount());
    EXPECT_EQ(64, left.columnCount());

    skelcl::Matrix<float> right(tmpright, 64);
    EXPECT_EQ(64, right.rowCount());
    EXPECT_EQ(64, right.columnCount());

    skelcl::Matrix<float> output = allpairs(left, right);
    EXPECT_EQ(64, output.rowCount());
    EXPECT_EQ(64, output.columnCount());

    for (size_t i = 0; i < output.rowCount(); ++i) {
        for (size_t j = 0; j < output.columnCount(); ++j) {
            float tmp = 0;
            for (size_t k = 0; k < left.columnCount(); ++k) {
                tmp += left[i][k] * right[k][j];
            }
            EXPECT_EQ(tmp, output[i][j]);
        }
    }
}

// Tests kernel with matrixmultiplication with one 32x128 matrix and one 128x32 matrix
TEST_F(AllPairsTest, NoSquareMatrices32x128SimpleAllPairs) {
    skelcl::Zip<float(float, float)> zip("float func(float x, float y){ return x*y; }");
    skelcl::Reduce<float(float)> reduce("float func(float x, float y){ return x+y; }");
    skelcl::AllPairs<float(float, float)> allpairs(reduce, zip);

    std::vector<float> tmpleft(4096);
    for (size_t i = 0; i < tmpleft.size(); ++i)
      tmpleft[i] = i;
    EXPECT_EQ(4096, tmpleft.size());

    std::vector<float> tmpright(tmpleft);
    EXPECT_EQ(4096, tmpright.size());

    skelcl::Matrix<float> left(tmpleft, 128);
    EXPECT_EQ(32, left.rowCount());
    EXPECT_EQ(128, left.columnCount());

    skelcl::Matrix<float> right(tmpright, 32);
    EXPECT_EQ(128, right.rowCount());
    EXPECT_EQ(32, right.columnCount());

    skelcl::Matrix<float> output = allpairs(left, right);
    EXPECT_EQ(32, output.rowCount());
    EXPECT_EQ(32, output.columnCount());

    for (size_t i = 0; i < output.rowCount(); ++i) {
        for (size_t j = 0; j < output.columnCount(); ++j) {
            float tmp = 0;
            for (size_t k = 0; k < left.columnCount(); ++k) {
                tmp += left[i][k] * right[k][j];
            }
            EXPECT_EQ(tmp, output[i][j]);
        }
    }
}

// Tests kernel with matrixmultiplication with two arbitrary matrices
TEST_F(AllPairsTest, ArbitraryMatricesAllPairs) {
    skelcl::Zip<float(float, float)> zip("float func(float x, float y){ return x*y; }");
    skelcl::Reduce<float(float)> reduce("float func(float x, float y){ return x+y; }");
    skelcl::AllPairs<float(float, float)> allpairs(reduce, zip);

    // M * N = D
    //----------------
    // M: height x dim
    // N: dim x width
    // D: height x width
    const unsigned int height = 100;
    const unsigned int dim = 2;
    const unsigned int width = 60;
    //----------------

    std::vector<float> tmpleft(height*dim);
    for (size_t i = 0; i < tmpleft.size(); ++i)
      tmpleft[i] = i % 100;
    EXPECT_EQ(height*dim, tmpleft.size());

    std::vector<float> tmpright(dim*width);
    for (size_t i = 0; i < tmpright.size(); ++i)
      tmpright[i] = i % 101;
    EXPECT_EQ(dim*width, tmpright.size());

    skelcl::Matrix<float> left(tmpleft, dim);
    EXPECT_EQ(height, left.rowCount());
    EXPECT_EQ(dim, left.columnCount());

    skelcl::Matrix<float> right(tmpright, width);
    EXPECT_EQ(dim, right.rowCount());
    EXPECT_EQ(width, right.columnCount());

    skelcl::Matrix<float> output = allpairs(left, right);
    EXPECT_EQ(height, output.rowCount());
    EXPECT_EQ(width, output.columnCount());

    for (size_t i = 0; i < output.rowCount(); ++i) {
        for (size_t j = 0; j < output.columnCount(); ++j) {
            float tmp = 0;
            for (size_t k = 0; k < left.columnCount(); ++k) {
                tmp += left[i][k] * right[k][j];
            }
            EXPECT_EQ(tmp, output[i][j]);
        }
    }
}
