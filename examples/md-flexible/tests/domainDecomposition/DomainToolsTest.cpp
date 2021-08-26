/**
 * @file DomainToolsTest.cpp
 * @author J. Körner
 * @date 27/05/21
 */
#include "DomainToolsTest.h"

#include "src/domainDecomposition/DomainTools.h"

TEST_F(DomainToolsTest, testIsInsideDomain) {
  std::array<double, 3> globalBoxMin = {1.0, 1.0, 1.0};
  std::array<double, 3> globalBoxMax = {10.0, 10.0, 10.0};

  std::array<double, 3> inside = {5.0, 5.0, 5.0};
  std::array<double, 3> onLowerBoundary = {1.0, 2.0, 5.0};
  std::array<double, 3> onUpperBoundary = {2.0, 10.0, 5.0};
  std::array<double, 3> outside = {0.0, 0.0, 0.0};

  EXPECT_EQ(true, DomainTools::isInsideDomain(inside, globalBoxMin, globalBoxMax));
  EXPECT_EQ(true, DomainTools::isInsideDomain(onLowerBoundary, globalBoxMin, globalBoxMax));
  EXPECT_EQ(false, DomainTools::isInsideDomain(onUpperBoundary, globalBoxMin, globalBoxMax));
  EXPECT_EQ(false, DomainTools::isInsideDomain(outside, globalBoxMin, globalBoxMax));
}
