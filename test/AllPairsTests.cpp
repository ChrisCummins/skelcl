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
TEST_F(AllPairsTest, SimpleAllPairs) {
    skelcl::Zip<float(float, float)> zip("float USR_ZIP(float x, float y){ return x*y; }");
    skelcl::Reduce<float(float)> reduce("float USR_REDUCE(float x, float y){ return x+y; }");
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
TEST_F(AllPairsTest, NotSoSimpleAllPairs) {
    skelcl::Zip<float(float, float)> zip("float USR_ZIP(float x, float y){ return x*y; }");
    skelcl::Reduce<float(float)> reduce("float USR_REDUCE(float x, float y){ return x+y; }");
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

// Tests kernel with matrixmultiplication
TEST_F(AllPairsTest, PotentialOutOfBoundsAllPairs) {
    skelcl::Zip<float(float, float)> zip("float USR_ZIP(float x, float y){ return x*y; }");
    skelcl::Reduce<float(float)> reduce("float USR_REDUCE(float x, float y){ return x+y; }");
    skelcl::AllPairs<float(float, float)> allpairs(reduce, zip);

    std::vector<float> tmpleft(400);
    for (size_t i = 0; i < tmpleft.size(); ++i)
      tmpleft[i] = i;
    EXPECT_EQ(400, tmpleft.size());

    std::vector<float> tmpright(tmpleft);
    EXPECT_EQ(400, tmpright.size());

    skelcl::Matrix<float> left(tmpleft, 20);
    EXPECT_EQ(20, left.rowCount());
    EXPECT_EQ(20, left.columnCount());

    skelcl::Matrix<float> right(tmpright, 20);
    EXPECT_EQ(20, right.rowCount());
    EXPECT_EQ(20, right.columnCount());

    skelcl::Matrix<float> output = allpairs(left, right);
    EXPECT_EQ(20, output.rowCount());
    EXPECT_EQ(20, output.columnCount());

    int errorCount = 0;
    for (size_t i = 0; i < output.rowCount(); ++i) {
        for (size_t j = 0; j < output.columnCount(); ++j) {
            float tmp = 0;
            for (size_t k = 0; k < left.columnCount(); ++k) {
                tmp += left[i][k] * right[k][j];
            }
            if (tmp != output[i][j]) {
                ++errorCount;
                //std::cout << "\\fill[color=red!30] (" << (40-i) <<"," << j << ") rectangle (" << (41-i) <<"," << j+1 <<");" << std::endl;
            }
            EXPECT_EQ(tmp, output[i][j]);
        }
    }
    //std::cout << "Error Count: " << errorCount << std::endl;
}
