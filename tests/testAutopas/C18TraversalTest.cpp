/**
 * @file C18TraversalTest.cpp
 * @author S. Seckler
 * @date 10.01.2019
 */

#include "C18TraversalTest.h"

using ::testing::_;
using ::testing::AtLeast;

TEST_F(C18TraversalTest, testTraversalCube) {
  std::array<size_t, 3> edgeLength = {2, 2, 2};

  MFunctor functor;
  std::vector<FPCell> cells;
  cells.resize(edgeLength[0] * edgeLength[1] * edgeLength[2]);
  autopas::Particle defaultParticle;

  GridGenerator::fillWithParticles(cells, edgeLength, defaultParticle);
#ifdef AUTOPAS_OPENMP
  int numThreadsBefore = omp_get_max_threads();
  omp_set_num_threads(4);
#endif
  autopas::C18Traversal<FPCell, MFunctor, false, true> C18Traversal(edgeLength, &functor);

  size_t boundaryXYcells = (edgeLength[2] - 1);
  size_t boundaryXYinteractions = boundaryXYcells * (7 + 5 + 6 + 4);
  size_t faceXcells = (edgeLength[1] - 2) * (edgeLength[2] - 1);
  size_t boundaryXinteractions = faceXcells * (9 + 8);
  size_t faceYcells = (edgeLength[0] - 2) * (edgeLength[2] - 1);
  size_t boundaryYinteractions = faceYcells * (7 + 10);
  size_t innercells = (edgeLength[0] - 2) * (edgeLength[1] - 2) * (edgeLength[2] - 2);
  size_t innercellinteractions = innercells * 13;

  EXPECT_CALL(functor, AoSFunctor(_, _, true))
      .Times(boundaryXYinteractions + boundaryXinteractions + boundaryYinteractions + innercellinteractions);

  C18Traversal.traverseCellPairs(cells);
#ifdef AUTOPAS_OPENMP
  omp_set_num_threads(numThreadsBefore);
#endif
}

TEST_F(C18TraversalTest, testTraversal2x2x2) {
  std::array<size_t, 3> edgeLength = {2, 2, 2};

  MFunctor functor;
  std::vector<FPCell> cells;
  cells.resize(edgeLength[0] * edgeLength[1] * edgeLength[2]);
  autopas::Particle defaultParticle;

  GridGenerator::fillWithParticles<autopas::Particle>(cells, edgeLength, defaultParticle);
#ifdef AUTOPAS_OPENMP
  int numThreadsBefore = omp_get_max_threads();
  omp_set_num_threads(4);
#endif
  autopas::C18Traversal<FPCell, MFunctor, false, true> C18Traversal(edgeLength, &functor);

  size_t boundaryXYcells = (edgeLength[2] - 1);
  size_t boundaryXYinteractions = boundaryXYcells * (7 + 5 + 6 + 4);
  size_t faceXcells = (edgeLength[1] - 2) * (edgeLength[2] - 1);
  size_t boundaryXinteractions = faceXcells * (9 + 8);
  size_t faceYcells = (edgeLength[0] - 2) * (edgeLength[2] - 1);
  size_t boundaryYinteractions = faceYcells * (7 + 10);
  size_t innercells = (edgeLength[0] - 2) * (edgeLength[1] - 2) * (edgeLength[2] - 2);
  size_t innercellinteractions = innercells * 13;

  EXPECT_CALL(functor, AoSFunctor(_, _, true))
      .Times(boundaryXYinteractions + boundaryXinteractions + boundaryYinteractions + innercellinteractions);

  C18Traversal.traverseCellPairs(cells);
#ifdef AUTOPAS_OPENMP
  omp_set_num_threads(numThreadsBefore);
#endif
}

TEST_F(C18TraversalTest, testTraversal2x3x4) {
  std::array<size_t, 3> edgeLength = {2, 3, 4};

  MFunctor functor;
  std::vector<FPCell> cells;
  cells.resize(edgeLength[0] * edgeLength[1] * edgeLength[2]);
  autopas::Particle defaultParticle;

  GridGenerator::fillWithParticles<autopas::Particle>(cells, {edgeLength[0], edgeLength[1], edgeLength[2]},
                                                      defaultParticle);
#ifdef AUTOPAS_OPENMP
  int numThreadsBefore = omp_get_max_threads();
  omp_set_num_threads(4);
#endif
  autopas::C18Traversal<FPCell, MFunctor, false, true> C18Traversal({edgeLength[0], edgeLength[1], edgeLength[2]},
                                                                    &functor);

  size_t boundaryXYcells = (edgeLength[2] - 1);
  size_t boundaryXYinteractions = boundaryXYcells * (7 + 5 + 6 + 4);
  size_t faceXcells = (edgeLength[1] - 2) * (edgeLength[2] - 1);
  size_t boundaryXinteractions = faceXcells * (9 + 8);
  size_t faceYcells = (edgeLength[0] - 2) * (edgeLength[2] - 1);
  size_t boundaryYinteractions = faceYcells * (7 + 10);
  size_t innercells = (edgeLength[0] - 2) * (edgeLength[1] - 2) * (edgeLength[2] - 2);
  size_t innercellinteractions = innercells * 13;

  EXPECT_CALL(functor, AoSFunctor(_, _, true))
      .Times(boundaryXYinteractions + boundaryXinteractions + boundaryYinteractions + innercellinteractions);

  C18Traversal.traverseCellPairs(cells);
#ifdef AUTOPAS_OPENMP
  omp_set_num_threads(numThreadsBefore);
#endif
}