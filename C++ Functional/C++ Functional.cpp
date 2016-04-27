#include "stdafx.h"
#include <chrono>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>

#include <thread>
#include <stdexcept>

using namespace std;
using namespace std::this_thread; // sleep_for, sleep_until

auto split(const string& str, char delim, vector<string>& output)
{
  auto i = 0;
  auto pos = str.find(delim);

  while (pos != string::npos)
  {
    output.emplace_back(str.substr(i, pos - i));
    i = ++pos;
    pos = str.find(delim, pos);

    if (pos == string::npos) 
    {
      output.emplace_back(str.substr(i, str.length()));
    }
  }

  return output.size();
}

using Pixels = vector<int>;

class Observation
{
public:
  Observation(string label, Pixels pixels)
  {
    _label = label;
    _pixels = pixels;
  }

  Observation()
  {
    _label.clear();
    _pixels.clear();
  }

  Pixels pixels() { return _pixels; }
  string label() { return _label; }

private:
  string _label;
  Pixels _pixels;
};

//_________________________________________________________________________________________________

using Observations = vector<Observation>;

auto ObservationData = [](vector<string>& csvData)
{
  Observations observations;

  for (size_t line = 0; line < csvData.size() - 1; ++line)
  {
    vector<string> columns;
    split(csvData[line], ',', columns);

    auto label = columns[0];
    Pixels pixels;

    for (size_t i = 1; i < columns.size(); ++i)
    {
      pixels.emplace_back(atoi(columns[i].c_str()));
    }

    observations.emplace_back(Observation(label, pixels));
  }

  return observations;
};

//_________________________________________________________________________________________________

auto readAllLines = [](string path)
{
  ifstream file(path.c_str());
  stringstream buffer;

  if (!file)
  {
    puts("File does not exist");
  }

  buffer << file.rdbuf();

  string str = buffer.str();
  vector<string> lines;

  split(str, '\n', lines);
  return lines;
};

//_________________________________________________________________________________________________


int main()
{
  auto reader = [](string path)
  {
    auto allLines = readAllLines(path);

    allLines.erase(allLines.begin()); // Skip CSV header.
    return ObservationData(allLines);
  };

  //_________________________________________________________________________________________________

  auto trainingPath = R"(../Data/trainingsample.csv)";
  auto trainingData = reader(trainingPath);

  auto validationPath = R"(../Data/validationsample.csv)";
  auto validationData = reader(validationPath);

  //_________________________________________________________________________________________________

  auto ManhattanDistance = [](Pixels pixels1, Pixels pixels2)
  {
    if (pixels1.size() != pixels2.size())
    {
      throw std::length_error("Inconsistent image sizes.");
    }

    auto length = pixels1.size();
    auto distance = 0;

    for (size_t i = 0; i < length; i++)
    {
      distance += abs(pixels1[i] - pixels2[i]);
    }

    return distance;
  };

  //_________________________________________________________________________________________________

  auto classify = [](Observations trainingSet, function <int(Pixels pixels1, Pixels pixels2)>& dist, Pixels pixels)
  {
    return 1;
  };


  return 0;
}

