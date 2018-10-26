/**
 * @file VerletClusterLists.h
 * @author nguyen
 * @date 14.10.18
 */

#pragma once

#include <cmath>
#include "autopas/containers/ParticleContainer.h"
#include "autopas/utils/ArrayMath.h"

namespace autopas {

/**
 * Particles are divided into clusters
 * The VerletClusterLists class uses neighborhood lists for each cluster
 * to calculate pairwise interactions of particles.
 * It is optimized for a constant, i.e. particle independent, cutoff radius of
 * the interaction.
 * @tparam Particle
 */
template <class Particle>
class VerletClusterLists : public ParticleContainer<Particle, FullParticleCell<Particle>> {
  /**
   * the index type to access the particle cells
   */
  typedef std::size_t index_t;

 public:
  /**
   * Constructor of the VerletClusterLists class.
   * The neighbor lists are build using a estimated density.
   * The box is divided into cuboids with roughly with approximately the
   * same side length. The rebuildFrequency should be chosen, s.t. the particles do
   * not move more than a distance of skin/2 between two rebuilds of the lists.
   * @param boxMin the lower corner of the domain
   * @param boxMax the upper corner of the domain
   * @param cutoff the cutoff radius of the interaction
   * @param skin the skin radius
   * @param rebuildFrequency specifies after how many pair-wise traversals the
   * neighbor lists are to be rebuild. A frequency of 1 means that they are
   * always rebuild, 10 means they are rebuild after 10 traversals
   * @param clusterSize size of clusters
   */
  VerletClusterLists(const std::array<double, 3> boxMin, const std::array<double, 3> boxMax, double cutoff,
                     double skin = 0, unsigned int rebuildFrequency = 1, int clusterSize = 4)
      : ParticleContainer<Particle, FullParticleCell<Particle>>(boxMin, boxMax, cutoff + skin),
        _clusterSize(clusterSize),
        _boxMin(boxMin),
        _boxMax(boxMax),
        _skin(skin),
        _cutoff(cutoff),
        _cutoffSqr(cutoff * cutoff),
        _traversalsSinceLastRebuild(UINT_MAX),
        _rebuildFrequency(rebuildFrequency),
        _neighborListIsValid(false) {
    rebuild();
  }

  ContainerOptions getContainerType() override { return ContainerOptions::verletListsCells; }

  /**
   * Function to iterate over all pairs of particles.
   * This function only handles short-range interactions.
   * @tparam the type of ParticleFunctor
   * @tparam Traversal
   * @param f functor that describes the pair-potential
   * @param traversal not used
   * @param useNewton3 whether newton 3 optimization should be used
   */
  template <class ParticleFunctor, class Traversal>
  void iteratePairwiseAoS(ParticleFunctor* f, Traversal* traversal, bool useNewton3 = true) {
    if (needsRebuild()) {  // if rebuild needed
      this->rebuild();
    }
    traverseVerletLists(f, useNewton3);

    // we iterated, so increase traversal counter
    _traversalsSinceLastRebuild++;
  }

  /**
   * Dummy function. (Uses AoS instead)
   * @tparam the type of ParticleFunctor
   * @tparam Traversal
   * @param f functor that describes the pair-potential
   * @param traversal not used
   * @param useNewton3 whether newton 3 optimization should be used
   */
  template <class ParticleFunctor, class Traversal>
  void iteratePairwiseSoA(ParticleFunctor* f, Traversal* traversal, bool useNewton3 = true) {
    iteratePairwiseAoS(f, traversal, useNewton3);
  }

  /**
   * @copydoc VerletLists::addParticle()
   */
  void addParticle(Particle& p) override {
    _neighborListIsValid = false;
    // add particle somewhere, because lists will be rebuild anyways
    _clusters[0].addParticle(p);
  }

  /**
   * @copydoc VerletLists::addHaloParticle()
   */
  void addHaloParticle(Particle& haloParticle) override { throw "VerletClusterLists.addHaloParticle not implemented"; }

  /**
   * @copydoc VerletLists::deleteHaloParticles
   */
  void deleteHaloParticles() override {}

  /**
   * @copydoc VerletLists::updateContainer()
   */
  void updateContainer() override {
    AutoPasLogger->debug("updating container");
    _neighborListIsValid = false;
  }

  bool isContainerUpdateNeeded() override { throw "VerletClusterLists.isContainerUpdateNeeded not implemented"; }

  TraversalSelector<FullParticleCell<Particle>> generateTraversalSelector(
      std::vector<TraversalOptions> traversalOptions) override {
    // at the moment this is just a dummy
    return TraversalSelector<FullParticleCell<Particle>>({0, 0, 0}, traversalOptions);
  }

  /**
   * specifies whether the neighbor lists need to be rebuild
   * @return true if the neighbor lists need to be rebuild, false otherwise
   */
  bool needsRebuild() {
    AutoPasLogger->debug("VerletLists: neighborlist is valid: {}", _neighborListIsValid);
    return (not _neighborListIsValid)                              // if the neighborlist is NOT valid a
                                                                   // rebuild is needed
           or (_traversalsSinceLastRebuild >= _rebuildFrequency);  // rebuild with frequency
  }

  ParticleIteratorWrapper<Particle> begin(IteratorBehavior behavior = IteratorBehavior::haloAndOwned) override {
    return ParticleIteratorWrapper<Particle>(
        new internal::ParticleIterator<Particle, FullParticleCell<Particle>>(&this->_clusters));
  }

  ParticleIteratorWrapper<Particle> getRegionIterator(
      std::array<double, 3> lowerCorner, std::array<double, 3> higherCorner,
      IteratorBehavior behavior = IteratorBehavior::haloAndOwned) override {
    throw "VerletClusterLists.getRegionIterator not implemented";
  }

 protected:
  /**
   * recalculate grids and clusters,
   * build verlet lists and
   * pad clusters
   */
  void rebuild() {
    // get all particles and clear clusters
    std::vector<Particle> invalidParticles;
    for (auto& cluster : _clusters) {
      for (auto it = cluster.begin(); it.isValid(); ++it) {
        invalidParticles.push_back(*it);
      }

      cluster.clear();
    }

    // get the dimensions and volumes of the box
    double boxSize[3]{};
    double volume = 1.0;
    for (int d = 0; d < 3; d++) {
      boxSize[d] = _boxMax[d] - _boxMin[d];
      volume *= boxSize[d];
    }

    if (invalidParticles.size() > 0) {
      // estimate particle density
      double density = invalidParticles.size() / volume;

      // guess optimal grid side length
      _gridSideLength = std::cbrt(_clusterSize / density);
    } else {
      _gridSideLength = std::max(boxSize[0], boxSize[1]);
    }
    _gridSideLengthReciprocal = 1 / _gridSideLength;

    // get cells per dimension
    index_t numCells = 1;
    for (int d = 0; d < 2; d++) {
      _cellsPerDim[d] = static_cast<index_t>(std::floor(boxSize[d] * _gridSideLengthReciprocal));
      // at least one cell
      _cellsPerDim[d] = std::max(_cellsPerDim[d], 1ul);

      numCells *= _cellsPerDim[d];
    }
    _cellsPerDim[2] = 1ul;

    // resize to number of grids
    _clusters.resize(numCells);
    _neighborLists.resize(numCells);

    // put particles into grids
    for (auto& particle : invalidParticles) {
      if (inBox(particle.getR(), _boxMin, _boxMax)) {
        auto index = getIndexOfPosition(particle.getR());
        _clusters[index].addParticle(particle);
      }
    }

    // sort by last dimension and reserve space for dummy particles
    for (auto& cluster : _clusters) {
      cluster.sortByZ();

      size_t size = cluster.numParticles();
      size_t rest = size % _clusterSize;
      if (rest > 0) cluster.reserve(size + (_clusterSize - rest));
    }

    // clear neighbor lists
    for (auto& verlet : _neighborLists) {
      verlet.clear();
    }

    updateVerletLists();
    padClusters();
  }

  /**
   * update the verlet lists
   */
  void updateVerletLists() {
    // the neighbor list is now valid
    _neighborListIsValid = true;
    _traversalsSinceLastRebuild = 0;

    const int boxRange = static_cast<int>(std::ceil((_cutoff + _skin) * _gridSideLengthReciprocal));

    const int gridMaxX = _cellsPerDim[0] - 1;
    const int gridMaxY = _cellsPerDim[1] - 1;
    for (int yi = 0; yi <= gridMaxY; yi++) {
      for (int xi = 0; xi <= gridMaxX; xi++) {
        auto& iGrid = _clusters[index1D(xi, yi)];
        // calculate number of full clusters and rest
        index_t iRest = iGrid.numParticles() % _clusterSize;
        index_t iSize = iGrid.numParticles() / _clusterSize;

        const int minX = std::max(xi - boxRange, 0);
        const int minY = std::max(yi - boxRange, 0);
        const int maxX = std::min(xi + boxRange, gridMaxX);
        const int maxY = std::min(yi + boxRange, gridMaxY);

        auto& iNeighbors = _neighborLists[index1D(xi, yi)];
        if (iRest > 0)
          iNeighbors.resize(iSize + 1);
        else
          iNeighbors.resize(iSize);
        for (int yj = minY; yj <= maxY; yj++) {
          double distY = std::max(0, std::abs(yi - yj) - 1) * _gridSideLength;
          for (int xj = minX; xj <= maxX; xj++) {
            auto& jGrid = _clusters[index1D(xj, yj)];

            // calculate number of  full clusters and rest
            index_t jRest = jGrid.numParticles() % _clusterSize;
            index_t jSize = jGrid.numParticles() / _clusterSize;

            // calculate distance in xy-plane and skip if already longer than cutoff
            double distX = std::max(0, std::abs(xi - xj) - 1) * _gridSideLength;
            double distXYsqr = distX * distX + distY * distY;
            if (distXYsqr <= _cutoffSqr) {
              for (index_t zi = 0; zi < iSize; zi++) {
                // bbox in z of iGrid
                float iBBoxBot = iGrid[zi * _clusterSize].getR()[2];
                float iBBoxTop = iGrid[(zi + 1) * _clusterSize - 1].getR()[2];
                auto& iClusterVerlet = iNeighbors[zi];
                for (index_t zj = 0; zj < jSize; zj++) {
                  // bbox in z of jGrid
                  Particle* jClusterStart = &jGrid[zj * _clusterSize];
                  float jBBoxBot = jClusterStart->getR()[2];
                  float jBBoxTop = (jClusterStart + (_clusterSize - 1))->getR()[2];

                  float distZ = bboxDistance(iBBoxBot, iBBoxTop, jBBoxBot, jBBoxTop);
                  if (distXYsqr + distZ * distZ <= _cutoffSqr) {
                    iClusterVerlet.push_back(jClusterStart);
                  }
                }
                // special case: last cluster not full
                if (jRest > 0) {
                  // bbox in z of jGrid
                  Particle* jClusterStart = &jGrid[jSize * _clusterSize];
                  float jBBoxBot = jClusterStart->getR()[2];
                  float jBBoxTop = (jClusterStart + (jRest - 1))->getR()[2];

                  float distZ = bboxDistance(iBBoxBot, iBBoxTop, jBBoxBot, jBBoxTop);
                  if (distXYsqr + distZ * distZ <= _cutoffSqr) {
                    iClusterVerlet.push_back(jClusterStart);
                  }
                }
              }
              // special case: last cluster not full
              if (iRest > 0) {
                // bbox in z of iGrid
                float iBBoxBot = iGrid[iSize * _clusterSize].getR()[2];
                float iBBoxTop = iGrid[iSize * _clusterSize + iRest - 1].getR()[2];
                auto& iClusterVerlet = iNeighbors[iSize];
                for (index_t zj = 0; zj < jSize; zj++) {
                  // bbox in z of jGrid
                  Particle* jClusterStart = &jGrid[zj * _clusterSize];
                  float jBBoxBot = jClusterStart->getR()[2];
                  float jBBoxTop = (jClusterStart + (_clusterSize - 1))->getR()[2];

                  float distZ = bboxDistance(iBBoxBot, iBBoxTop, jBBoxBot, jBBoxTop);
                  if (distXYsqr + distZ * distZ <= _cutoffSqr) {
                    iClusterVerlet.push_back(jClusterStart);
                  }
                }
                // special case: last cluster not full
                if (jRest > 0) {
                  // bbox in z of jGrid
                  Particle* jClusterStart = &jGrid[jSize * _clusterSize];
                  float jBBoxBot = jClusterStart->getR()[2];
                  float jBBoxTop = (jClusterStart + (jRest - 1))->getR()[2];

                  float distZ = bboxDistance(iBBoxBot, iBBoxTop, jBBoxBot, jBBoxTop);
                  if (distXYsqr + distZ * distZ <= _cutoffSqr) {
                    iClusterVerlet.push_back(jClusterStart);
                  }
                }
              }
            }
          }
        }
      }
    }
  }

  /**
   * Pad clusters with dummy particles
   */
  void padClusters() {
    for (index_t x = 0; x < _cellsPerDim[0]; x++) {
      for (index_t y = 0; y < _cellsPerDim[1]; y++) {
        auto& grid = _clusters[index1D(x, y)];
        index_t rest = grid.numParticles() % _clusterSize;
        if (rest > 0) {
          for (int i = rest; i < _clusterSize; i++) {
            Particle p = Particle();
            p.setR({2 * x * _cutoff, 2 * y * _cutoff, 2 * _boxMax[2] + 2 * i * _cutoff});
            grid.addParticle(p);
          }
        }
      }
    }
  }

  /**
   * Traverse over the verlet lists
   * @param functor Functor applied to each particle pair
   * @param useNewton3
   */
  template <class ParticleFunctor>
  void traverseVerletLists(ParticleFunctor* functor, bool useNewton3) {
    if (needsRebuild()) {
      rebuild();
    }

    for (index_t x = 0; x < _cellsPerDim[0]; x++) {
      for (index_t y = 0; y < _cellsPerDim[1]; y++) {
        index_t index = index1D(x, y);
        auto& grid = _clusters[index];
        auto& gridVerlet = _neighborLists[index];

        const index_t gridSize = grid.numParticles() / 4;
        for (index_t z = 0; z < gridSize; z++) {
          Particle* iClusterStart = &grid[z * 4];
          for (auto neighbor : gridVerlet[z]) {
            if (iClusterStart == neighbor) {
              // self pair
              for (int i = 0; i < _clusterSize; i++) {
                for (int j = i + 1; j < _clusterSize; j++) {
                  Particle* iParticle = iClusterStart + i;
                  Particle* jParticle = neighbor + j;
                  functor->AoSFunctor(*iParticle, *jParticle, useNewton3);
                  if (not useNewton3) functor->AoSFunctor(*jParticle, *iParticle, useNewton3);
                }
              }
            } else {
              for (int i = 0; i < _clusterSize; i++) {
                for (int j = 0; j < _clusterSize; j++) {
                  Particle* iParticle = iClusterStart + i;
                  Particle* jParticle = neighbor + j;
                  functor->AoSFunctor(*iParticle, *jParticle, useNewton3);
                }
              }
            }
          }
        }
      }
    }
  }

  /**
   * Calculates the distance of two bounding boxes in one dimension
   * @param min1 minimum coordinate of first bbox in tested dimension
   * @param max1 maximum coordinate of first bbox in tested dimension
   * @param min2 minimum coordinate of second bbox in tested dimension
   * @param max2 maximum coordinate of second bbox in tested dimension
   * @return distance
   */
  inline float bboxDistance(const float min1, const float max1, const float min2, const float max2) const {
    if (max1 < min2) {
      return min2 - max1;
    } else if (min1 > max2) {
      return min1 - max2;
    } else {
      return 0;
    }
  }

  /**
   * gets the grid containing a particle in given position
   * @param pos the position of the particle
   * @return the index of the grid
   */
  inline index_t getIndexOfPosition(const std::array<double, 3>& pos) const {
    std::array<index_t, 2> cellIndex{};

    for (int dim = 0; dim < 2; dim++) {
      const long int value = (static_cast<long int>(floor((pos[dim] - _boxMin[dim]) * _gridSideLengthReciprocal))) + 1l;
      const index_t nonnegativeValue = static_cast<index_t>(std::max(value, 0l));
      const index_t nonLargerValue = std::min(nonnegativeValue, _cellsPerDim[dim] - 1);
      cellIndex[dim] = nonLargerValue;
      /// @todo this is a sanity check to prevent doubling of particles, but
      /// could be done better!
      if (pos[dim] >= _boxMax[dim]) {
        cellIndex[dim] = _cellsPerDim[dim] - 1;
      } else if (pos[dim] < _boxMin[dim]) {
        cellIndex[dim] = 0;
      }
    }

    return cellIndex[0] + cellIndex[1] * _cellsPerDim[0];
    // in very rare cases rounding is stupid, thus we need a check...
    /// @todo when the border and flag manager is there
  }

  /**
   * converts grid position to the index in the vector
   * @param x x-position in grid
   * @param y y-position in grid
   * @return index in vector
   */
  inline index_t index1D(const index_t x, const index_t y) { return x + y * _cellsPerDim[0]; }

 private:
  // neighbors of clusters for each grid
  std::vector<std::vector<std::vector<Particle*>>> _neighborLists;

  /// internal storage, particles are split into a grid in xy-dimension
  std::vector<FullParticleCell<Particle>> _clusters;
  int _clusterSize;

  std::array<double, 3> _boxMin;
  std::array<double, 3> _boxMax;

  // side length of xy-grid and reciprocal
  double _gridSideLength;
  double _gridSideLengthReciprocal;

  // dimensions of grid
  std::array<index_t, 3> _cellsPerDim;

  /// skin radius
  double _skin;

  /// cutoff
  double _cutoff;
  double _cutoffSqr;

  /// how many pairwise traversals have been done since the last traversal
  unsigned int _traversalsSinceLastRebuild;

  /// specifies after how many pairwise traversals the neighbor list is to be
  /// rebuild
  unsigned int _rebuildFrequency;

  // specifies if the neighbor list is currently valid
  bool _neighborListIsValid;
};

}  // namespace autopas
