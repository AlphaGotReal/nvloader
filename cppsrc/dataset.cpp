#include <iostream>

#include "dataset.hpp"

namespace dataset {

Dataset::Dataset(const ManifestT& manifest) {
  std::cerr << "dataset::Dataset constructor" << std::endl;
}

size_t Dataset::size() const {
  return 0;
}

torch::Tensor Dataset::fetch(uint64_t idx) {
  return torch::zeros({3, 5, 3, 480, 640});
}

} // namespace dataset
