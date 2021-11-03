/**
 * @file ParticleView.h
 *
 * @date 2 Nov 2021
 * @author lgaertner
 */

#pragma once

#include <Kokkos_Core.hpp>

/**
 * ParticleView class.
 * This class uses a Kokkos::view to store particles and allows access to its references.
 * It also keeps the order of particles based on the currently used container.
 * @tparam ParticleType Type of the Particle that is being stored
 */
template <class ParticleType>
class ParticleView {

 public:
  ParticleView<ParticleType>() = default;

  void addParticle(const ParticleType &p) {
    _particleListLock.lock();

    if (_size == _capacity) {
      //resize
    }
    deep_copy(subview(_particleListImp, _size), p);
    _size++;

    _particleListLock.unlock();
  }

  void deleteAll() {

  }

  size_t getSize() const {
    return _size;
  }

 private:
  /**
   * Flag indicating whether there are out-of-date references in the vector.
   */
  bool _dirty = false;
//  size_t _dirtyIndex{0};

  const static size_t _INITIAL_CAPACITY{8};

  size_t _capacity{_INITIAL_CAPACITY};
  size_t _size{0};

  autopas::AutoPasLock _particleListLock;
  Kokkos::View<ParticleType[_INITIAL_CAPACITY]> _particleListImp;
};
