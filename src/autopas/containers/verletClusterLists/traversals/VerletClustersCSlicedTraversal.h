/**
 * @file VerletClustersCSlicedTraversal.h
 * @author fischerv
 * @date 11 Aug 2020
 */

#pragma once

#include "autopas/containers/cellPairTraversals/CSlicedBasedTraversal.h"
#include "autopas/containers/verletClusterLists/VerletClusterLists.h"
#include "autopas/containers/verletClusterLists/traversals/ClusterFunctor.h"
#include "autopas/containers/verletClusterLists/traversals/VerletClustersTraversalInterface.h"

namespace autopas {

/**
 * This traversal splits the domain into slices along the longer dimension among x and y.
 * The sliced are divided into two colors which are separately processed to prevent race
 * conditions.
 *
 * @tparam ParticleCell
 * @tparam PairwiseFunctor
 * @tparam dataLayout
 * @tparam useNewton3
 */
template <class ParticleCell, class PairwiseFunctor, DataLayoutOption::Value dataLayout, bool useNewton3>
class VerletClustersCSlicedTraversal
    : public CSlicedBasedTraversal<ParticleCell, PairwiseFunctor, dataLayout, useNewton3>,
      public VerletClustersTraversalInterface<typename ParticleCell::ParticleType> {
 private:
  using Particle = typename ParticleCell::ParticleType;

  PairwiseFunctor *_functor;
  internal::ClusterFunctor<Particle, PairwiseFunctor, dataLayout, useNewton3> _clusterFunctor;

  void processBaseStep(unsigned long x, unsigned long y) {
    std::cout << "x: " << x << "y: " << y << std::endl;
    auto &clusterList = *VerletClustersTraversalInterface<Particle>::_verletClusterLists;
    auto &currentTower = clusterList.getTowerAtCoordinates(x, y);
    for (auto &cluster : currentTower.getClusters()) {
      std::cout << "cluster: " << &cluster << std::endl;
      _clusterFunctor.traverseCluster(cluster);
      for (auto *neighborCluster : cluster.getNeighbors()) {
        std::cout << "neighbour: " << neighborCluster << std::endl;
        _clusterFunctor.traverseClusterPair(cluster, *neighborCluster);
      }
    }
  }

 public:
  /**
   * Constructor of the VerletClustersCSlicedTraversal.
   * @param dims The dimensions of the cellblock, i.e. the number of cells in x,
   * y and z direction.
   * @param pairwiseFunctor The functor to use for the traveral.
   * @param interactionLength Interaction length (cutoff + skin).
   * @param cellLength cell length.
   * @param clusterSize the number of particles per cluster.
   */
  explicit VerletClustersCSlicedTraversal(const std::array<unsigned long, 3> &dims, PairwiseFunctor *pairwiseFunctor,
                                          const double interactionLength, const std::array<double, 3> &cellLength,
                                          size_t clusterSize)
      : CSlicedBasedTraversal<ParticleCell, PairwiseFunctor, dataLayout, useNewton3>(dims, pairwiseFunctor,
                                                                                     interactionLength, cellLength),
        _functor(pairwiseFunctor),
        _clusterFunctor(pairwiseFunctor, clusterSize) {}

  [[nodiscard]] TraversalOption getTraversalType() const override { return TraversalOption::verletClustersCSliced; }

  [[nodiscard]] DataLayoutOption getDataLayout() const override { return dataLayout; }

  [[nodiscard]] bool getUseNewton3() const override { return useNewton3; }

  void initTraversal() override {
    if constexpr (dataLayout == DataLayoutOption::soa) {
      VerletClustersTraversalInterface<Particle>::_verletClusterLists->loadParticlesIntoSoAs(_functor);
    }
    CSlicedBasedTraversal<ParticleCell, PairwiseFunctor, dataLayout, useNewton3>::initTraversal();
  }

  void endTraversal() override {
    if constexpr (dataLayout == DataLayoutOption::soa) {
      VerletClustersTraversalInterface<Particle>::_verletClusterLists->extractParticlesFromSoAs(_functor);
    }
    CSlicedBasedTraversal<ParticleCell, PairwiseFunctor, dataLayout, useNewton3>::endTraversal();
  }

  void traverseParticlePairs() override {
    this->template cSlicedTraversal</*allCells*/ true>(
        [&](unsigned long x, unsigned long y, unsigned long z) { processBaseStep(x, y); });
  }
};
}  // namespace autopas
