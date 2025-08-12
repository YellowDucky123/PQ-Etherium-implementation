/// Function implementing hash chains, implemented over a tweakable hash function
/// The chain is specific to an epoch `epoch`, and an index `chain_index`. All
/// evaluations of the tweakable hash function use the given parameter `parameter`
/// and tweaks determined by `epoch`, `chain_index`, and their position in the chain.
/// We start walking the chain at position `start_pos_in_chain` with `start`,
/// and then walk the chain for `steps` many steps. For example, walking two steps
/// with `start = A` would mean we walk A -> B -> C, and then return C.

template <typename TH>
typename TH::Domain chain(typename TH::Parameter parameter, uint32_t epoch, uint8_t chain_index, uint8_t start_pos_in_chain, unsigned int steps, typename TH::Domain &start)
{
      // keep track of what we have
      auto current = start;

      // otherwise, walk the right amount of steps
      for (size_t j = 0; j < steps; j++)
      {
            auto tweak = TH::chain_tweak(epoch, chain_index, start_pos_in_chain + static_cast<uint8_t>(j));

            current = TH::aply(parameter, tweak, current);
      }
      // return where we are now
      return current;
};