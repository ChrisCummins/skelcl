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
/// \author Michel Steuwer <michel.steuwer@uni-muenster.de>
///

#include <gtest/gtest.h>

#include <fstream>

#include <cstdio>

#include <SkelCL/SkelCL.h>
#include <SkelCL/Vector.h>
#include <SkelCL/Map.h>
#include <SkelCL/detail/Logger.h>

class MapTest : public ::testing::Test {
protected:
  MapTest() {
    // skelcl::detail::defaultLogger.setLoggingLevel(skelcl::detail::Logger::Severity::Debug);
    skelcl::init(skelcl::nDevices(1));
  };

  ~MapTest() {
    skelcl::terminate();
  };
};

TEST_F(MapTest, CreateMapWithString) {
  skelcl::Map<float(float)> m {"float func(float f){ return -f; }"};
}

TEST_F(MapTest, CreateMapWithFile) {
  std::string filename{ "TestFunction.cl" };
  {
    std::ofstream file{ filename, std::ios_base::trunc };
    file << "float func(float f) { return -f; }";
  }
  std::ifstream file{ filename };

  skelcl::Map<float(float)> m{ file };

  remove(filename.c_str()); // delete file
}

TEST_F(MapTest, CreateMapWithFileII) {
  std::string filename{ "TestFunction.cl" };
  {
    std::ofstream file{ filename, std::ios_base::trunc };
    file << "float func(float f) { return -f; }";
  }

  skelcl::Map<float(float)> m{ std::ifstream{ filename } };

  remove(filename.c_str()); // delete file
}

TEST_F(MapTest, SimpleMap) {
  skelcl::Map<float(float)> m{ "float func(float f){ return -f; }" };

  skelcl::Vector<float> input(10);
  for (size_t i = 0; i < input.size(); ++i) {
    input[i] = i * 2.5f;
  }
  EXPECT_EQ(10, input.size());

  skelcl::Vector<float> output = m(input);

  EXPECT_EQ(10, output.size());
  for (size_t i = 0; i < output.size(); ++i) {
    EXPECT_EQ(-input[i], output[i]);
  }
}

TEST_F(MapTest, SimpleMultiDeviceMap) {
  skelcl::terminate();
  skelcl::init(skelcl::nDevices(2));
  skelcl::Map<float(float)> m{ "float func(float f){ return -f; }" };

  skelcl::Vector<float> input(10);
  for (size_t i = 0; i < input.size(); ++i) {
    input[i] = i * 2.5f;
  }
  EXPECT_EQ(10, input.size());

  skelcl::Vector<float> output(input.size());
  m(skelcl::out(output), input);

  EXPECT_EQ(10, output.size());
  for (size_t i = 0; i < output.size(); ++i) {
    EXPECT_EQ(-input[i], output[i]);
  }
}

TEST_F(MapTest, AddArgs) {
  skelcl::Map<float(float)> m{ "float func(float f, float add, float add2)\
                                 { return f+add+add2; }" };

  skelcl::Vector<float> input(10);
  for (size_t i = 0; i < input.size(); ++i) {
    input[i] = i * 2.5f;
  }
  EXPECT_EQ(10, input.size());

  float add  = 5.25f;
  float add2 = 7.75f;

  skelcl::Vector<float> output = m(input, add, add2);

  EXPECT_EQ(10, output.size());
  for (size_t i = 0; i < output.size(); ++i) {
    EXPECT_EQ(input[i]+add+add2, output[i]);
  }
}

TEST_F(MapTest, MapVoid) {
  skelcl::Map<void(float)> m{ "void func(float f, __global float* out) { out[get_global_id(0)] = f; }" };

  skelcl::Vector<float> input(10);
  for (size_t i = 0; i < input.size(); ++i) {
    input[i] = i * 2.5f;
  }
  EXPECT_EQ(10, input.size());

  skelcl::Vector<float> output(10);
  m(input, skelcl::out(output));

  for (size_t i = 0; i < output.size(); ++i) {
    EXPECT_EQ(input[i], output[i]);
  }
}

skelcl::Vector<float> execute(const skelcl::Vector<float>& input)
{
  skelcl::Map<float(float)> m{ "float func(float f) { return -f; }" };
  // tmp should not be destroyed right away
  // the computation must finish first
  skelcl::Vector<float> tmp{ input };
  return m(tmp);
}

TEST_F(MapTest, TempInputVector) {
  auto size = 1024 * 100;
  skelcl::Vector<float> output;
  skelcl::Vector<float> input(size);
  for (size_t i = 0; i < input.size(); ++i) {
    input[i] = i * 2.5f;
  }
  EXPECT_EQ(size, input.size());

  output = execute(input);

  for (size_t i = 0; i < output.size(); ++i) {
    EXPECT_EQ(-input[i], output[i]);
  }
}
