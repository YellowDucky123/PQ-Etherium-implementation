template <typename TH>
typename TH::Domain chain(TH::Parameter, uint32_t epoch, uint8_t chain_index, uint8_t start_pos_in_chain, unsigned int steps, TH::DOMAIN &start)
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