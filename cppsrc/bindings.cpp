#include <memory>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <torch/extension.h>

#include "dataset.hpp"

namespace py = pybind11;

PYBIND11_MODULE(_nvloader, m) {
  py::class_<dataset::Dataset>(m, "Dataset")
    .def(py::init<const dataset::Dataset::ManifestT&>())
    .def( "__len__", &dataset::Dataset::size)
    .def("fetch", &dataset::Dataset::fetch);
}
