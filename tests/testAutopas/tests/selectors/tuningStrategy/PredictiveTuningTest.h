/**
 * @file PredictiveTuningTest.h
 * @author Julian Pelloth
 * @date 01.04.2020
 */

#pragma once

#include <gtest/gtest.h>

#include "AutoPasTestBase.h"
#include "autopas/selectors/tuningStrategy/PredictiveTuning.h"

class PredictiveTuningTest : public AutoPasTestBase {
 protected:
  const autopas::Configuration configurationC01 =
      autopas::Configuration(autopas::ContainerOption::linkedCells, 1., autopas::TraversalOption::c01,
                             autopas::DataLayoutOption::soa, autopas::Newton3Option::disabled);
  const autopas::Configuration configurationC08 =
      autopas::Configuration(autopas::ContainerOption::linkedCells, 1., autopas::TraversalOption::c08,
                             autopas::DataLayoutOption::soa, autopas::Newton3Option::disabled);
  const autopas::Configuration configurationSliced =
      autopas::Configuration(autopas::ContainerOption::linkedCells, 1., autopas::TraversalOption::sliced,
                             autopas::DataLayoutOption::soa, autopas::Newton3Option::disabled);
};
