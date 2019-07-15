#include "ParticleClassLibrary.h"

//@todo entferne unused functions
ParticleClassLibrary::ParticleClassLibrary(map<unsigned long, double> &sigma, map<unsigned long, double> &epsilon,
                                           map<unsigned long, double> &mass)
    : Epsilon(epsilon), Sigma(sigma), Mass(mass) {}

ParticleClassLibrary::ParticleClassLibrary() {}

ParticleClassLibrary::ParticleClassLibrary(const ParticleClassLibrary &pcl)
    : Epsilon(pcl.Epsilon), Sigma(pcl.Sigma), Mass(pcl.Mass) {}

double ParticleClassLibrary::get24Epsilon(unsigned long i) { return 24 * Epsilon.at(i); }

double ParticleClassLibrary::getSSigma(unsigned long i) {
  double sigma = Sigma.at(i);
  return sigma * sigma;
}

double ParticleClassLibrary::getMass(Particle i) { return Mass.at(i.getID()); }

double ParticleClassLibrary::mixing24E(unsigned long i, unsigned long j) {
  return 24 * sqrt(Epsilon.at(i) * Epsilon.at(j));
}

double ParticleClassLibrary::mixingSS(unsigned long i, unsigned long j) {
  double mixingS = (Sigma.at(i) + Sigma.at(j)) / 2;
  return mixingS * mixingS;
}