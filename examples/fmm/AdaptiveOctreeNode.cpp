/**
 * @file AdaptiveOctreeNode.cpp
 * @date 30.10.19
 * @author Joachim Marin
 */

#include "AdaptiveOctreeNode.h"
#include "AdaptiveOctree.h"

AdaptiveOctreeNode::AdaptiveOctreeNode(AdaptiveOctree &tree, AdaptiveOctreeNode *parent, int childIndex,
                                       std::array<double, 3> minCorner, std::array<double, 3> maxCorner)
    : tree(&tree),
      parent(parent),
      nodeMinCorner(minCorner),
      nodeCenter(autopas::ArrayMath::mulScalar(autopas::ArrayMath::add(minCorner, maxCorner), 0.5)),
      nodeMaxCorner(maxCorner),
      nodeSize(autopas::ArrayMath::sub(maxCorner, minCorner)) {
  // Initialize expansion coefficients to 0.
  fmmM = ComplexMatrix(tree.getOrderOfExpansion() * 2 + 1, std::vector<Complex>(tree.getOrderOfExpansion() + 1, 0));
  fmmL = ComplexMatrix(tree.getOrderOfExpansion() * 2 + 1, std::vector<Complex>(tree.getOrderOfExpansion() + 1, 0));

  // Node name:
  if (parent == nullptr) {
    depth = 0;
    name = "root";
  } else {
    depth = parent->depth + 1;
    name = parent->name + "->" + std::to_string(childIndex);
  }

  int particles = tree.getNumberOfParticlesInRegion(nodeMinCorner, nodeMaxCorner);

  /*std::cout << "[" << name << "] node at (" << minCorner[0] << ", " << minCorner[1] << ", " << minCorner[2] << ") to
  ("
            << maxCorner[0] << ", " << maxCorner[1] << ", " << maxCorner[2] << ") with depth " << depth
            << " containing " << particles << " particles. size = " << nodeSize[0] << ", " << nodeSize[1] << ", "
            << nodeSize[2] << std::endl;

  if (particles <= tree.getMaxParticlesPerNode()) {
    for (auto particle = getTree()->getDomain()->getRegionIterator(minCorner, maxCorner); particle.isValid();
         ++particle) {
      std::cout << particle->getR()[0] << ", " << particle->getR()[1] << ", " << particle->getR()[2]
                << ", charge = " << particle->charge << std::endl;
    }
  }*/

  // Only divide node, if depth < minDepth or (enough particles and depth < maxDepth)
  if (depth < tree.getMinDepth() ||
      ((particles > tree.getMaxParticlesPerNode()) && (depth < tree.getMaxDepth() || tree.getMaxDepth() == -1))) {
    _isLeaf = false;

    child = std::vector<std::unique_ptr<AdaptiveOctreeNode>>(8);

    // Divide node into 8 smaller nodes.
    for (int i = 0; i < 8; ++i) {
      // Each bit defines whether it is in the first or second half of a dimension.
      auto u = static_cast<unsigned>(i);
      bool xOffset = u & 0x1u;
      bool yOffset = u & 0x2u;
      bool zOffset = u & 0x4u;
      std::array<double, 3> offset = std::array<double, 3>(
          {xOffset ? nodeSize[0] / 2 : 0, yOffset ? nodeSize[1] / 2 : 0, zOffset ? nodeSize[2] / 2 : 0});
      child[i] = std::make_unique<AdaptiveOctreeNode>(tree, this, i, autopas::ArrayMath::add(nodeMinCorner, offset),
                                                      autopas::ArrayMath::add(nodeCenter, offset));
    }
  } else {
    _isLeaf = true;
    tree.currentMaxDepth = std::max(tree.currentMaxDepth, depth);
    tree.numberOfLeaves++;
  }
  tree.numberOfNodes++;
}
AdaptiveOctreeNode *AdaptiveOctreeNode::findNode(const std::array<double, 3> &position, int maxDepth) const {
  if (isLeaf() || depth == maxDepth) {
    return const_cast<AdaptiveOctreeNode *>(this);
  } else {
    int childIndex = 0;
    double centerX = nodeCenter[0];
    double centerY = nodeCenter[1];
    double centerZ = nodeCenter[2];
    double posX = position[0];
    double posY = position[1];
    double posZ = position[2];
    if (posX >= centerX) {
      childIndex += 1;
    }
    if (posY >= centerY) {
      childIndex += 2;
    }
    if (posZ >= centerZ) {
      childIndex += 4;
    }
    return child[childIndex]->findNode(position, maxDepth);
  }
}
AdaptiveOctreeNode *AdaptiveOctreeNode::findNeighbour(int x, int y, int z) const {
  // Neighbour cells are never smaller than the current cell.
  // So when searching at nodeSize away from the center, it will always be the neighbour cell.

  std::array<double, 3> offset = std::array<double, 3>({x * nodeSize[0], y * nodeSize[1], z * nodeSize[2]});

  auto neighbour = tree->getRoot()->findNode(autopas::ArrayMath::add(nodeCenter, offset), depth);
  // std::cout << "find = (" << nodeCenter[0] + offset[0] << ", " << nodeCenter[1] + offset[1] << ", "
  //          << nodeCenter[2] + offset[2] << ") -> " << neighbour->name << std::endl;

  return neighbour;
}

Complex AdaptiveOctreeNode::getM(int m, int n) {
  int offset = m <= 0 ? 0 : -1;
  int indexM = 2 * std::abs(m) + offset;
  if (indexM < 0 || indexM >= static_cast<int>(fmmM.size())) {
    return 0.0;
  }
  if (n < 0 || n >= static_cast<int>(fmmM[0].size())) {
    return 0.0;
  }
  return fmmM.at(indexM).at(n);
}

void AdaptiveOctreeNode::setM(int m, int n, Complex value) {
  assert(!__isnan(value.real()) && !__isnan(value.imag()));
  int offset = m <= 0 ? 0 : -1;
  fmmM.at(2 * std::abs(m) + offset).at(n) = value;

  if (value != 0.0) {
    _isZeroM = false;
  }
}

Complex AdaptiveOctreeNode::getL(int m, int n) {
  int offset = m <= 0 ? 0 : -1;
  int indexM = 2 * std::abs(m) + offset;
  if (indexM < 0 || indexM >= static_cast<int>(fmmL.size())) {
    return 0.0;
  }
  if (n < 0 || n >= static_cast<int>(fmmL[0].size())) {
    return 0.0;
  }
  return fmmL.at(indexM).at(n);
}

void AdaptiveOctreeNode::setL(int m, int n, Complex value) {
  assert(!__isnan(value.real()) && !__isnan(value.imag()));
  int offset = m <= 0 ? 0 : -1;
  fmmL.at(2 * std::abs(m) + offset).at(n) = value;

  if (value != 0.0) {
    _isZeroL = false;
  }
}
void AdaptiveOctreeNode::initNeighbourList() {
  // Neighbours are the smallest nodes that are not smaller than this node and are adjacent to it.
  neighbourList = std::set<AdaptiveOctreeNode *>();
  neighbourList.insert(this);
  for (int x = -1; x <= 1; ++x) {
    for (int y = -1; y <= 1; ++y) {
      for (int z = -1; z <= 1; ++z) {
        neighbourList.insert(findNeighbour(x, y, z));
      }
    }
  }
  neighbourListString = "[" + name + "] neighbourList: (";
  for (auto neighbour : neighbourList) {
    neighbourListString += neighbour->name + ", ";
  }
  neighbourListString += ")";

  if (!_isLeaf) {
    for (int c = 0; c < 8; ++c) {
      child[c]->initNeighbourList();
    }
  }
}
void AdaptiveOctreeNode::initNearFieldList() {
  nearFieldList = std::set<AdaptiveOctreeNode *>();
  for (auto *neighbour : neighbourList) {
    nearFieldList.insert(neighbour);
  }
  nearFieldListString = "[" + name + "] nearFieldList: (";
  for (auto nearField : nearFieldList) {
    nearFieldListString += nearField->name + ", ";
    getTree()->totalNearFieldNodes++;
  }

  if (!_isLeaf) {
    for (int c = 0; c < 8; ++c) {
      child[c]->initNearFieldList();
    }
  }
}

void AdaptiveOctreeNode::initInteractionList() {
  interactionList = std::set<AdaptiveOctreeNode *>();
  if (parent != nullptr) {
    for (AdaptiveOctreeNode *parentNeighbour : parent->getNeighbourList()) {
      if (!parentNeighbour->isLeaf() && parentNeighbour->depth == parent->depth) {
        for (int c = 0; c < 8; ++c) {
          AdaptiveOctreeNode *insert;
          insert = parentNeighbour->getChild(c);
          if (neighbourList.find(insert) == neighbourList.end()) {
            interactionList.insert(insert);
          }
        }
      } else {
        if (neighbourList.find(parentNeighbour) == neighbourList.end()) {
          if (nearFieldList.find(parentNeighbour) == nearFieldList.end()) {
            nearFieldList.insert(parentNeighbour);
            nearFieldListString += parentNeighbour->name + ", ";
            getTree()->totalNearFieldNodes++;
          }
        }
      }
    }
  }

  nearFieldListString += ")";

  interactionListString = "[" + name + "] interactionList: (";
  for (auto inter : interactionList) {
    interactionListString += inter->name + ", ";
    getTree()->totalInteractionNodes++;
  }
  interactionListString += ")";

  if (!_isLeaf) {
    for (int c = 0; c < 8; ++c) {
      child[c]->initInteractionList();
    }
  }
}