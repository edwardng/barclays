#pragma once

#include <string>
#include <vector>

namespace basket::pricer {
struct CSVReader {
  using RowData = std::vector<std::vector<std::string>>;

  explicit CSVReader(const std::string &filename) : csv_path_(filename) {
  };

  const RowData getData() const;

 private:
  const std::string csv_path_{""};
};
}