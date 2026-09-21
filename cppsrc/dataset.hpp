#pragma once

#include <map>
#include <vector>
#include <string>

#include <torch/torch.h>

namespace dataset {

class Dataset {
public:

  using ManifestT = std::vector<std::vector<std::map<std::string, uint64_t>>>;

  Dataset() = delete;
  Dataset(const ManifestT& manifest);

  size_t size() const;

  torch::Tensor fetch(uint64_t idx);

};

} // namespace dataset
