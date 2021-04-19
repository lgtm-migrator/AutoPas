/**
 * @file MDFlexSingleNode.h
 * @author J. Körner
 * @date 07.04.2021
 */

#include "MDFlexSingleNode.h"

#if defined(AUTOPAS_MPI)
#include <mpi.h>
#endif

#include <iostream>
#include <fstream>

MDFlexSingleNode::MDFlexSingleNode(int argc, char** argv) : MDFlexSimulation(argc, argv){
	#if defined(AUTOPAS_INTERNODE_TUNING)
  	MPI_Init(&argc, &argv);
  	int rank;
  	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  	std::cout << "rank: " << rank << std::endl;
	#endif
}

MDFlexSingleNode::~MDFlexSingleNode(){
  // print config.yaml file of current run
  if (_configuration->dontCreateEndConfig.value) {
    std::ofstream configFileEnd("MDFlex_end_" + autopas::utils::Timer::getDateStamp() + ".yaml");
    configFileEnd << "# Generated by:" << std::endl;
    configFileEnd << "# ";
    for (int i = 0; i < _argc; ++i) {
      configFileEnd << _argv[i] << " ";
    }
    configFileEnd << std::endl;
    configFileEnd << *_configuration;
    configFileEnd.close();
  }
#if defined(AUTOPAS_INTERNODE_TUNING)
  MPI_Finalize();
#endif
}

void MDFlexSingleNode::run(){
	std::streambuf* streamBuffer;
	std::ofstream logFile;
	// select either std::out or a logfile for autopas log output.
 	// This does not affect md-flex output.
 	if (_configuration->logFileName.value.empty()) {
   	streamBuffer = std::cout.rdbuf();
 	} else {
   	logFile.open(_configuration->logFileName.value);
   	streamBuffer = logFile.rdbuf();
 	}
 	std::ostream outputStream(streamBuffer);

 	// print config to console
 	std::cout << *_configuration;

  // Initialization. Use particle type from the Simulation class.
  autopas::AutoPas<Simulation::ParticleType> autopas(outputStream);
  _simulation->initialize(*_configuration, autopas);

  std::cout << std::endl << "Using " << autopas::autopas_get_max_threads() << " Threads" << std::endl;

	std::cout << "Starting simulation... " << std::endl;
	_simulation->simulate(autopas);
  std::cout << "Simulation done!" << std::endl << std::endl;

  // Statistics about the simulation
  _simulation->printStatistics(autopas);
}
