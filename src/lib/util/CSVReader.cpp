#include "CSVReader.h"

#include <fstream>

#include <boost/algorithm/string.hpp>

namespace basket::pricer {
const CSVReader::RowData CSVReader::getData() const {
  RowData rowdata;

  std::ifstream ifs;
  ifs.open(csv_path_);

  std::string line{""};
  while (getline(ifs, line)) {
	std::vector<std::string> row;
	boost::split(row, line, boost::is_any_of(","));
	rowdata.push_back(row);
  }

  ifs.close();

  return rowdata;
}
}