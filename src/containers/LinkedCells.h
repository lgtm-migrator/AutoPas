/*
 * LinkedCells.h
 *
 *  Created on: 17 Jan 2018
 *      Author: tchipevn
 */

#ifndef AUTOPAS_SRC_CONTAINERS_LINKEDCELLS_H_
#define AUTOPAS_SRC_CONTAINERS_LINKEDCELLS_H_

#include "CellBlock3D.h"
#include "ParticleContainer.h"
#include "containers/cellPairTraversals/SlicedTraversal.h"
#include "utils/inBox.h"

namespace autopas {

template<class Particle, class ParticleCell>
class LinkedCells : public ParticleContainer<Particle, ParticleCell> {
 public:
  LinkedCells(const std::array<double, 3> boxMin,
              const std::array<double, 3> boxMax, double cutoff)
      : ParticleContainer<Particle, ParticleCell>(boxMin, boxMax, cutoff),
        _cellBlock(this->_data, boxMin, boxMax, cutoff) {}

  void addParticle(Particle &p) override {
    bool inBox = autopas::inBox(p.getR(), this->getBoxMin(), this->getBoxMax());
    if (inBox) {
      ParticleCell &cell = _cellBlock.getContainingCell(p.getR());
      cell.addParticle(p);
    } else {
      // todo
    }
  }

  void iteratePairwiseAoS(Functor<Particle, ParticleCell> *f) override {
    iteratePairwiseAoS2(f);
  }

  template<class ParticleFunctor>
  void iteratePairwiseAoS2(ParticleFunctor *f) {
    CellFunctor<Particle, ParticleCell, ParticleFunctor, false> cellFunctor(f);
    //		cellFunctor.processCellAoSN3(this->_data[13]);
    SlicedTraversal<ParticleCell,
                    CellFunctor<Particle, ParticleCell, ParticleFunctor, false>>
        traversal(this->_data, _cellBlock.getCellsPerDimensionWithHalo(),
                  &cellFunctor);
    traversal.traverseCellPairs();
  }

  void iteratePairwiseSoA(Functor<Particle, ParticleCell> *f) override {
    //TODO: iteratePairwiseSoA
    iteratePairwiseSoA2(f);
  }

  template<class ParticleFunctor>
  void iteratePairwiseSoA2(ParticleFunctor *f) {
    CellFunctor<Particle, ParticleCell, ParticleFunctor, true> cellFunctor(f);
    //		cellFunctor.processCellAoSN3(this->_data[13]);
    SlicedTraversal<ParticleCell,
                    CellFunctor<Particle, ParticleCell, ParticleFunctor, true>>
        traversal(this->_data, _cellBlock.getCellsPerDimensionWithHalo(),
                  &cellFunctor);
    traversal.traverseCellPairs();
  }

 private:
  CellBlock3D<ParticleCell> _cellBlock;
  // ThreeDimensionalCellHandler
};

} /* namespace autopas */

#endif /* AUTOPAS_SRC_CONTAINERS_LINKEDCELLS_H_ */
