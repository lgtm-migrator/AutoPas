/**
 * @file KokkosDSTraversalInterface.h
 * @author lgaertner
 * @date 10.11.21
 */

#pragma once

namespace autopas {

/**
 * Interface for traversals used by the KokkosDirectSum container.
 *
 * The container only accepts traversals in its iteratePairwise() method that implement this interface.
 * @tparam ParticleCell
 */
template <class ParticleCell>
class KokkosDSTraversalInterface {};

}  // namespace autopas
