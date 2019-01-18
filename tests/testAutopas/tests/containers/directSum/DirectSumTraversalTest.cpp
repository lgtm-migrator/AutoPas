/**
 * @file DirectSumTraversalTest.cpp
 * @author F. Gratl
 * @date 11/23/18
 */

#include "DirectSumTraversalTest.h"
#include "autopas/containers/directSum/DirectSumTraversal.h"
#include "testingHelpers/RandomGenerator.h"

using ::testing::_;
using ::testing::AtLeast;

TEST_F(DirectSumTraversalTest, testTraversalAoS) { testTraversal(false); }

TEST_F(DirectSumTraversalTest, testTraversalSoA) { testTraversal(true); }

void DirectSumTraversalTest::testTraversal(bool useSoA) {
  size_t numParticles = 20;
  size_t numHaloParticles = 10;

  MFunctor functor;
  std::vector<FPCell> cells;
  cells.resize(2);
  autopas::Particle defaultParticle;

  Particle particle;
  for (size_t i = 0; i < numParticles + numHaloParticles; ++i) {
    particle.setID(i);
    // first particles go in domain cell rest to halo cell
    if (i < numParticles) {
      particle.setR(RandomGenerator::randomPosition({0, 0, 0}, {10, 10, 10}));
      cells[0].addParticle(particle);
    } else {
      particle.setR(RandomGenerator::randomPosition({10, 10, 10}, {20, 20, 20}));
      cells[1].addParticle(particle);
    }
  }

  if (useSoA) {
    autopas::DirectSumTraversal<FPCell, MFunctor, true, true> traversal(&functor);
    // domain SoA with itself
    EXPECT_CALL(functor, SoAFunctor(_, true)).Times(1);
    // domain SoA with halo
    EXPECT_CALL(functor, SoAFunctor(_, _, true)).Times(1);
    traversal.traverseCellPairs(cells);
  } else {
    autopas::DirectSumTraversal<FPCell, MFunctor, false, true> traversal(&functor);
    // interactions in main cell + interactions with halo.
    size_t expectedFunctorCalls = numParticles * (numParticles - 1) / 2 + numParticles * numHaloParticles;
    EXPECT_CALL(functor, AoSFunctor(_, _, true)).Times((int)expectedFunctorCalls);
    traversal.traverseCellPairs(cells);
  }
}