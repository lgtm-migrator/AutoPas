/**
 * @file TimeDiscretization.h
 * @author N. Fottner
 * @date 13/05/19
 */
#pragma once
#include "autopas/AutoPas.h"
#include "autopas/utils/ArrayMath.h"
#include "autopas/options/IteratorBehavior.h"

/**
 * Functions for updating velocities and positions as simulation time progresses.
 */
namespace TimeDiscretization {

/**
 * Calculate and update the position for every particle using the Störmer-Verlet Algorithm.
 * @param autopas
 * @param particlePropertiesLibrary
 * @param deltaT time step width
 */
template <class AutoPasTemplate, class ParticlePropertiesLibraryTemplate>
void calculatePositions(AutoPasTemplate &autopas, const ParticlePropertiesLibraryTemplate &particlePropertiesLibrary,
                        const double deltaT) {
  // helper declarations for operations with vector
  using autopas::utils::ArrayMath::add;
  using autopas::utils::ArrayMath::mulScalar;

  auto forEachLambda = [&](auto &particle) {
    auto v = particle.getV();
    auto m = particlePropertiesLibrary.getMass(particle.getTypeId());
    auto f = particle.getF();
    particle.setOldF(f);
    particle.setF({0., 0., 0.});
    v = mulScalar(v, deltaT);
    f = mulScalar(f, (deltaT * deltaT / (2 * m)));
    auto newR = add(v, f);
    particle.addR(newR);
  };
  autopas.forEach(forEachLambda, autopas::IteratorBehavior::owned);
}

/**
 * Calculate and update the velocity for every particle using the the Störmer-Verlet Algorithm.
 * @param autopas
 * @param particlePropertiesLibrary
 * @param deltaT time step width
 */
template <class AutoPasTemplate, class ParticlePropertiesLibraryTemplate>
void calculateVelocities(AutoPasTemplate &autopas, const ParticlePropertiesLibraryTemplate &particlePropertiesLibrary,
                         const double deltaT) {
  // helper declarations for operations with vector
  using autopas::utils::ArrayMath::add;
  using autopas::utils::ArrayMath::mulScalar;

  auto forEachLambda = [&](auto &particle) {
    auto m = particlePropertiesLibrary.getMass(particle.getTypeId());
    auto force = particle.getF();
    auto oldForce = particle.getOldf();
    auto newV = mulScalar((add(force, oldForce)), deltaT / (2 * m));
    particle.addV(newV);
  };
  autopas.forEach(forEachLambda, autopas::IteratorBehavior::owned);
}

}  // namespace TimeDiscretization
