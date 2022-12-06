/**
* @file MultisiteLJFunctorTest.h
* @author S. Newcome
* @date 12/05/2022
*/

#pragma once

#include <gtest/gtest.h>

#include "AutoPasTestBase.h"
#include "autopas/AutoPasDecl.h"
#include "autopas/molecularDynamics/LJFunctor.h"
#include "autopas/molecularDynamics/LJMultisiteFunctor.h"
#include "autopas/molecularDynamics/MultisiteMoleculeLJ.h"
#include "autopas/molecularDynamics/ParticlePropertiesLibrary.h"

/**
 * Test class for MulticenteredLJFunctor
 */
class MultisiteLJFunctorTest : public AutoPasTestBase {
 public:
  /**
   * Constructor
   */
  MultisiteLJFunctorTest() = default;

  /**
   * Generates a particle properties library for use with ::generateMolecules
   * @param PPL
   */
  void generatePPL(ParticlePropertiesLibrary<double, size_t> *PPL);
  /**
   * Generates a reproducible set of molecules of varying numbers of molecules.
   * @param molecules
   * @param offset position offset
   */
  void generateMolecules(std::vector<autopas::MultisiteMoleculeLJ> *molecules, std::array<double, 3> offset);

  /**
   * Tests the correctness of the AoS functor for a given molA, molB, PPL, and cutoff.
   * @tparam newton3
   * @param molA
   * @param molB
   * @param PPL
   * @param cutoff
   */
  template <bool newton3>
  void testAoSForceCalculation(autopas::MultisiteMoleculeLJ molA, autopas::MultisiteMoleculeLJ molB, ParticlePropertiesLibrary<double, size_t> PPL, double cutoff);

  /**
   * Compares the forces produced by the single-site functor and the multi-site functor applied to single-site molecules.
   * @tparam newton3
   * @param molA
   * @param molB
   * @param PPL
   * @param cutoff
   */
  template <bool newton3>
  void singleSiteSanityCheck(autopas::MultisiteMoleculeLJ molA, autopas::MultisiteMoleculeLJ molB, ParticlePropertiesLibrary<double, size_t> PPL, double cutoff);

  /**
   * Compares the correctness of the SoACellFunctor against that of the AoSFunctor.
   * @tparam newton3
   * @param molecules vector of multi-site molecules
   * @param PPL Particle Properties Library
   * @param cutoff
   */
  template <bool newton3>
  void testSoACellAgainstAoS(std::vector<autopas::MultisiteMoleculeLJ> molecules, ParticlePropertiesLibrary<double, size_t> PPL, double cutoff);

  /**
   * Compares the correctness of the SoACellPairFunctor against that of the AoSFunctor.
   * @tparam newton3
   * @param moleculesA
   * @param moleculesB
   * @param PPL
   * @param cutoff
   */
  template <bool newton3>
  void testSoACellPairAgainstAoS(std::vector<autopas::MultisiteMoleculeLJ> moleculesA, std::vector<autopas::MultisiteMoleculeLJ> moleculesB, ParticlePropertiesLibrary<double, size_t> PPL, double cutoff);

  /**
   * Compares the correctness of the SoAVerletFunctor against that of the AoSFunctor.
   * @tparam newton3
   * @param molecules
   * @param PPL
   * @param cutoff
   */
  template <bool newton3>
  void testSoAVerletAgainstAoS(std::vector<autopas::MultisiteMoleculeLJ> molecules, ParticlePropertiesLibrary<double, size_t> PPL, double cutoff);
};

