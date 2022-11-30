/**
 * @file ParticleSerializationTools.cpp
 * @author J. Körner
 * @date 13.05.2021
 */
#include "ParticleSerializationTools.h"

#include <tuple>

namespace {

/**
 * Stores the AttributeNames of the attributes of ParticleType which have to be communicated using MPI.
 */
constexpr std::array<typename autopas::MoleculeLJ::AttributeNames, 15> SingleSiteAttributes = {
    autopas::MoleculeLJ::AttributeNames::id,
    autopas::MoleculeLJ::AttributeNames::posX,
    autopas::MoleculeLJ::AttributeNames::posY,
    autopas::MoleculeLJ::AttributeNames::posZ,
    autopas::MoleculeLJ::AttributeNames::velocityX,
    autopas::MoleculeLJ::AttributeNames::velocityY,
    autopas::MoleculeLJ::AttributeNames::velocityZ,
    autopas::MoleculeLJ::AttributeNames::forceX,
    autopas::MoleculeLJ::AttributeNames::forceY,
    autopas::MoleculeLJ::AttributeNames::forceZ,
    autopas::MoleculeLJ::AttributeNames::oldForceX,
    autopas::MoleculeLJ::AttributeNames::oldForceY,
    autopas::MoleculeLJ::AttributeNames::oldForceZ,
    autopas::MoleculeLJ::AttributeNames::typeId,
    autopas::MoleculeLJ::AttributeNames::ownershipState};

/**
 * Stores the AttributeNames of the attributes of ParticleType which have to be communicated using MPI.
 */
constexpr std::array<typename autopas::MultisiteMoleculeLJ::AttributeNames, 25> MultiSiteAttributes = {
    autopas::MultisiteMoleculeLJ::AttributeNames::id,
    autopas::MultisiteMoleculeLJ::AttributeNames::posX,
    autopas::MultisiteMoleculeLJ::AttributeNames::posY,
    autopas::MultisiteMoleculeLJ::AttributeNames::posZ,
    autopas::MultisiteMoleculeLJ::AttributeNames::velocityX,
    autopas::MultisiteMoleculeLJ::AttributeNames::velocityY,
    autopas::MultisiteMoleculeLJ::AttributeNames::velocityZ,
    autopas::MultisiteMoleculeLJ::AttributeNames::forceX,
    autopas::MultisiteMoleculeLJ::AttributeNames::forceY,
    autopas::MultisiteMoleculeLJ::AttributeNames::forceZ,
    autopas::MultisiteMoleculeLJ::AttributeNames::oldForceX,
    autopas::MultisiteMoleculeLJ::AttributeNames::oldForceY,
    autopas::MultisiteMoleculeLJ::AttributeNames::oldForceZ,
    autopas::MultisiteMoleculeLJ::AttributeNames::quaternion0,
    autopas::MultisiteMoleculeLJ::AttributeNames::quaternion1,
    autopas::MultisiteMoleculeLJ::AttributeNames::quaternion2,
    autopas::MultisiteMoleculeLJ::AttributeNames::quaternion3,
    autopas::MultisiteMoleculeLJ::AttributeNames::angularVelX,
    autopas::MultisiteMoleculeLJ::AttributeNames::angularVelY,
    autopas::MultisiteMoleculeLJ::AttributeNames::angularVelZ,
    autopas::MultisiteMoleculeLJ::AttributeNames::torqueX,
    autopas::MultisiteMoleculeLJ::AttributeNames::torqueY,
    autopas::MultisiteMoleculeLJ::AttributeNames::torqueZ,
    autopas::MultisiteMoleculeLJ::AttributeNames::typeId,
    autopas::MultisiteMoleculeLJ::AttributeNames::ownershipState};

/**
 * The combined size in byte of the simple attributes which need to be communicated using MPI.
 */
constexpr size_t singleSiteAttributesSize = 120;

/**
 * The combined size in byte of the rotational attributes which need to be communicated using MPI.
 */
constexpr size_t multiSitelAttributesSize = 200;

/**
 * Serializes the attribute of a molecule defined by I.
 * @tparam isMultiSite: Flag for if simulation is multi-site.
 * @param particle: The particle who's attribute needs to be serialized.
 * @param attributeVector: The container in which the serialized attribute will be stored.
 * @param startIndex: The startindex in the container where to store the serialized attribute.
 */
template <bool isMultiSite, size_t I>
void serializeAttribute(const autopas::MoleculeLJ &particle, std::vector<char> &attributeVector, size_t &startIndex) {
  const auto attribute = isMultiSite ? particle.get<MultiSiteAttributes[I]>() : particle.get<SingleSiteAttributes[I]>();
  const auto sizeOfValue = sizeof(attribute);
  std::memcpy(&attributeVector[startIndex], &attribute, sizeOfValue);
  startIndex += sizeOfValue;
}

/**
 * Deserializes the attribute of simple molecule defined by I.
 * @tparam isMultiSite: Flag for if simulation is multi-site.
 * @param attributeVector: The vector containing the data which needs to be deserialized.
 * @param particle: The particle to which the serialized data will be applied.
 * @param startIndex: The start index in the attributeVector of the attribute which needs to be deserialized.
 */
template <bool isMultiSite, size_t I>
void deserializeAttribute(char *&attributeVector, autopas::MoleculeLJ &particle, size_t &startIndex) {
  const auto attribute = isMultiSite ? particle.get<MultiSiteAttributes[I]>() : particle.get<SingleSiteAttributes[I]>();
  const auto sizeOfValue = sizeof(attribute);
  std::memcpy(&attribute, &attributeVector[startIndex], sizeOfValue);
  if constexpr (isMultiSite) {
    particle.set<MultiSiteAttributes[I]>(attribute);
  } else {
    particle.set<SingleSiteAttributes[I]>(attribute);
  }
  startIndex += sizeOfValue;
}

/**
 * The implementation of serializeParticle using the expansion operator.
 * @param particle: The particle which will be serialized.
 * @param serializedParticle: The char array of the particles serialized attributes.
 */
template <size_t... I, class ParticleClass, bool isMultiSite>
void serializeParticleImpl(const ParticleClass &particle, std::vector<char> &serializedParticle,
                           std::index_sequence<I...>) {
  // Serialize particle attributes
  size_t startIndex = 0;
  std::vector<char> attributesVector(isMultiSite ? multiSitelAttributesSize : singleSiteAttributesSize);
  (serializeAttribute<I, isMultiSite>(particle, attributesVector, startIndex), ...);

  // Add serialized attributes to serialized particle
  serializedParticle.insert(serializedParticle.end(), attributesVector.begin(), attributesVector.end());
}

/**
 * The implementation of deserializeParticle using the expansion operator.
 * @param particleData: The particle data which will be deserialized.
 * @param particle: The particle to which the deserialized attributes will be applied.
 */
template <size_t... I, class ParticleClass, bool isMultiSite>
void deserializeParticleImpl(char *particleData, ParticleClass &particle, std::index_sequence<I...>) {
  size_t startIndex = 0;
  (deserializeAttribute<I, isMultiSite>(particleData, particle, startIndex), ...);
}
}  // namespace

namespace ParticleSerializationTools {
template <bool isMultiSite, class ParticleClass>
void serializeParticle(const ParticleClass &particle, std::vector<char> &serializedParticles) {
  if constexpr (isMultiSite) {
    serializeParticleImpl<ParticleClass, isMultiSite>(particle, serializedParticles, std::make_index_sequence<MultiSiteAttributes.size()>{});
  } else {
    serializeParticleImpl<ParticleClass, isMultiSite>(particle, serializedParticles, std::make_index_sequence<SingleSiteAttributes.size()>{});
  }
}

template <bool isMultiSite, class ParticleClass>
void deserializeParticle(char *particleData, ParticleClass &particle) {
  if constexpr (isMultiSite) {
    deserializeParticleImpl<ParticleClass, isMultiSite>(particleData, particle, std::make_index_sequence<MultiSiteAttributes.size()>{});
  } else {
    deserializeParticleImpl<ParticleClass, isMultiSite>(particleData, particle, std::make_index_sequence<SingleSiteAttributes.size()>{});
  }

}

template <bool isMultiSite, class ParticleClass>
void deserializeParticles(std::vector<char> &particlesData, std::vector<ParticleClass> &particles) {
  ParticleClass particle;
  for (size_t i = 0; i < particlesData.size(); i += isMultiSite ? multiSitelAttributesSize : singleSiteAttributesSize) {
    deserializeParticle<ParticleClass, isMultiSite>(&particlesData[i], particle);
    particles.push_back(particle);
  }
}

}  // namespace ParticleSerializationTools