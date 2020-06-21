/**
 * @file AutoPasConfigurationCommunicatorTest.h
 * @author W. Thieme
 * @date 05.06.2020
 */

#include "AutoPasConfigurationCommunicatorTest.h"

using namespace autopas::utils::AutoPasConfigurationCommunicator;
using namespace autopas;

TEST_F(AutoPasConfigurationCommunicatorTest, testSerializeAndDeserialize) {
  Configuration config = Configuration(ContainerOption::directSum, 1.2, TraversalOption::sliced,
                                       LoadEstimatorOption::none, DataLayoutOption::cuda, Newton3Option::disabled);
  Configuration passedConfig = deserializeConfiguration(serializeConfiguration(config));
  EXPECT_EQ(passedConfig, config);
}

TEST_F(AutoPasConfigurationCommunicatorTest, testOptimizeConfiguration) {
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  Configuration config =
      Configuration(ContainerOption::directSum, 1 + rank, TraversalOption::sliced,
                    LoadEstimatorOption::neighborListLength, DataLayoutOption::aos, Newton3Option::enabled);
  Configuration optimized = optimizeConfiguration(MPI_COMM_WORLD, config, rank);

  EXPECT_EQ(optimized,
            Configuration(ContainerOption::directSum, 1, TraversalOption::sliced,
                          LoadEstimatorOption::neighborListLength, DataLayoutOption::aos, Newton3Option::enabled));
}

TEST_F(AutoPasConfigurationCommunicatorTest, testDistributeConfigurationsFiniteCellSizeFactors) {
  std::set<ContainerOption> containerOptions{ContainerOption::verletClusterLists, ContainerOption::linkedCells};
  NumberSetFinite<double> cellSizeFactors{0.9, 1.0, 1.1};
  std::set<TraversalOption> traversalOptions{TraversalOption::verletClusters, TraversalOption::sliced};
  std::set<LoadEstimatorOption> loadEstimatorOptions{LoadEstimatorOption::none,
                                                     LoadEstimatorOption::squaredParticlesPerCell};
  std::set<DataLayoutOption> dataLayoutOptions{DataLayoutOption::aos, DataLayoutOption::soa};
  std::set<Newton3Option> newton3Options{Newton3Option::enabled, Newton3Option::disabled};
  int rank, commSize;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &commSize);

  int totalNumConfigsBefore = getSearchSpaceSize(containerOptions, cellSizeFactors, traversalOptions,
                                                 loadEstimatorOptions, dataLayoutOptions, newton3Options);
  distributeConfigurations(containerOptions, cellSizeFactors, traversalOptions, loadEstimatorOptions, dataLayoutOptions,
                           newton3Options, rank, commSize);
  int totalNumConfigsAfter = getSearchSpaceSize(containerOptions, cellSizeFactors, traversalOptions,
                                                loadEstimatorOptions, dataLayoutOptions, newton3Options);

  if (commSize <= totalNumConfigsBefore) {
    EXPECT_GE(totalNumConfigsAfter, totalNumConfigsBefore / commSize);
  } else {
    EXPECT_EQ(totalNumConfigsAfter, 1);
  }
}

// tests the precise outcomes of each rank for a fictional commSize of 4
TEST_F(AutoPasConfigurationCommunicatorTest, testDistributeConfigurationsFiniteCellSizeFactorsFakeMPI) {
  std::set<ContainerOption> containerOptions{ContainerOption::verletClusterLists, ContainerOption::linkedCells};
  NumberSetFinite<double> cellSizeFactors{0.9, 1.0, 1.1};
  std::set<TraversalOption> traversalOptions{TraversalOption::verletClusters, TraversalOption::sliced};
  std::set<LoadEstimatorOption> loadEstimatorOptions{LoadEstimatorOption::none};
  std::set<DataLayoutOption> dataLayoutOptions{DataLayoutOption::aos, DataLayoutOption::soa};
  std::set<Newton3Option> newton3Options{Newton3Option::enabled, Newton3Option::disabled};

  // Rank 0
  auto containersTmp = std::set<ContainerOption>(containerOptions);
  auto cellSizeFactorsTmp = NumberSetFinite<double>(cellSizeFactors);
  auto traversalsTmp = std::set<TraversalOption>(traversalOptions);
  auto loadEstimatorTmp = std::set<LoadEstimatorOption>(loadEstimatorOptions);
  auto dataLayoutTmp = std::set<DataLayoutOption>(dataLayoutOptions);
  auto newton3Tmp = std::set<Newton3Option>(newton3Options);
  auto firstAndSecondCellSizes = std::set<double>{0.9, 1.0};
  auto secondAndThirdCellSizes = std::set<double>{1.0, 1.1};

  distributeConfigurations(containersTmp, cellSizeFactorsTmp, traversalsTmp, loadEstimatorTmp, dataLayoutTmp,
                           newton3Tmp, 0, 4);
  EXPECT_EQ(containersTmp, std::set<ContainerOption>{ContainerOption::linkedCells});
  EXPECT_EQ(cellSizeFactorsTmp.getAll(), firstAndSecondCellSizes);
  EXPECT_EQ(traversalsTmp, std::set<TraversalOption>{TraversalOption::sliced});
  EXPECT_EQ(loadEstimatorTmp, std::set<LoadEstimatorOption>{LoadEstimatorOption::none});
  EXPECT_EQ(dataLayoutTmp, dataLayoutOptions);
  EXPECT_EQ(newton3Tmp, newton3Options);

  // Rank 1
  containersTmp = std::set<ContainerOption>(containerOptions);
  cellSizeFactorsTmp = NumberSetFinite<double>(cellSizeFactors);
  traversalsTmp = std::set<TraversalOption>(traversalOptions);
  loadEstimatorTmp = std::set<LoadEstimatorOption>(loadEstimatorOptions);
  dataLayoutTmp = std::set<DataLayoutOption>(dataLayoutOptions);
  newton3Tmp = std::set<Newton3Option>(newton3Options);

  distributeConfigurations(containersTmp, cellSizeFactorsTmp, traversalsTmp, loadEstimatorTmp, dataLayoutTmp,
                           newton3Tmp, 1, 4);
  EXPECT_EQ(containersTmp, std::set<ContainerOption>{ContainerOption::linkedCells});
  EXPECT_EQ(cellSizeFactorsTmp.getAll(), secondAndThirdCellSizes);
  EXPECT_EQ(traversalsTmp, std::set<TraversalOption>{TraversalOption::sliced});
  EXPECT_EQ(loadEstimatorTmp, std::set<LoadEstimatorOption>{LoadEstimatorOption::none});
  EXPECT_EQ(dataLayoutTmp, dataLayoutOptions);
  EXPECT_EQ(newton3Tmp, newton3Options);

  // Rank 2
  containersTmp = std::set<ContainerOption>(containerOptions);
  cellSizeFactorsTmp = NumberSetFinite<double>(cellSizeFactors);
  traversalsTmp = std::set<TraversalOption>(traversalOptions);
  loadEstimatorTmp = std::set<LoadEstimatorOption>(loadEstimatorOptions);
  dataLayoutTmp = std::set<DataLayoutOption>(dataLayoutOptions);
  newton3Tmp = std::set<Newton3Option>(newton3Options);

  distributeConfigurations(containersTmp, cellSizeFactorsTmp, traversalsTmp, loadEstimatorOptions, dataLayoutTmp,
                           newton3Tmp, 2, 4);
  EXPECT_EQ(containersTmp, std::set<ContainerOption>{ContainerOption::verletClusterLists});
  EXPECT_EQ(cellSizeFactorsTmp.getAll(), firstAndSecondCellSizes);
  EXPECT_EQ(traversalsTmp, std::set<TraversalOption>{TraversalOption::verletClusters});
  EXPECT_EQ(loadEstimatorTmp, std::set<LoadEstimatorOption>{LoadEstimatorOption::none});
  EXPECT_EQ(dataLayoutTmp, dataLayoutOptions);
  EXPECT_EQ(newton3Tmp, newton3Options);

  // Rank 3
  containersTmp = std::set<ContainerOption>(containerOptions);
  cellSizeFactorsTmp = NumberSetFinite<double>(cellSizeFactors);
  traversalsTmp = std::set<TraversalOption>(traversalOptions);
  loadEstimatorTmp = std::set<LoadEstimatorOption>(loadEstimatorOptions);
  dataLayoutTmp = std::set<DataLayoutOption>(dataLayoutOptions);
  newton3Tmp = std::set<Newton3Option>(newton3Options);

  distributeConfigurations(containersTmp, cellSizeFactorsTmp, traversalsTmp, loadEstimatorTmp, dataLayoutTmp,
                           newton3Tmp, 3, 4);
  EXPECT_EQ(containersTmp, std::set<ContainerOption>{ContainerOption::verletClusterLists});
  EXPECT_EQ(cellSizeFactorsTmp.getAll(), secondAndThirdCellSizes);
  EXPECT_EQ(traversalsTmp, std::set<TraversalOption>{TraversalOption::verletClusters});
  EXPECT_EQ(loadEstimatorTmp, std::set<LoadEstimatorOption>{LoadEstimatorOption::none});
  EXPECT_EQ(dataLayoutTmp, dataLayoutOptions);
  EXPECT_EQ(newton3Tmp, newton3Options);
}

TEST_F(AutoPasConfigurationCommunicatorTest, testDistributeConfigurationsInfiniteCellSizeFactors) {
  std::set<ContainerOption> containerOptions{ContainerOption::verletClusterLists};
  NumberInterval<double> cellSizeFactors{0.8, 1.2};
  std::set<TraversalOption> traversalOptions{TraversalOption::verletClusters};
  std::set<LoadEstimatorOption> loadEstimatorOptions{LoadEstimatorOption::squaredParticlesPerCell};
  std::set<DataLayoutOption> dataLayoutOptions{DataLayoutOption::aos};
  std::set<Newton3Option> newton3Options{Newton3Option::enabled};
  int rank, commSize;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &commSize);

  distributeConfigurations(containerOptions, cellSizeFactors, traversalOptions, loadEstimatorOptions, dataLayoutOptions,
                           newton3Options, rank, commSize);

  EXPECT_FALSE(containerOptions.empty() or cellSizeFactors.isEmpty() or traversalOptions.empty() or
               dataLayoutOptions.empty() or newton3Options.empty());
  double error = 0.001;
  EXPECT_GE(cellSizeFactors.getMin(), 0.8 + (0.4 / commSize) * rank - error);
  EXPECT_LE(cellSizeFactors.getMin(), 0.8 + (0.4 / commSize) * rank + error);
  EXPECT_GE(cellSizeFactors.getMax(), 0.8 + (0.4 / commSize) * (rank + 1) - error);
  EXPECT_LE(cellSizeFactors.getMax(), 0.8 + (0.4 / commSize) * (rank + 1) + error);
}
