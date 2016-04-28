#include "stdafx.h"
#include <chrono>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

#include <thread>
#include <stdexcept>

using namespace std;
using namespace std::this_thread; // sleep_for, sleep_until

// split function
auto split(const string& str, char delim, vector<string>& output) noexcept
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
      auto finalString = str.substr(i, str.length());

      if (!finalString.empty())
      {
        output.emplace_back();
      }
    }
  }

  return output.size();
}

// readAllLines lambda
auto readAllLines = [](string& path)
{
  ifstream file(path.c_str());
  stringstream buffer;

  if (!file)
  {
    // Any 'throw' type is good for the moment...
    throw std::invalid_argument(" **** File does not exist");
  }

  buffer << file.rdbuf();

  string str = buffer.str();
  vector<string> lines;

  split(str, '\n', lines);

  return lines;
};

//_________________________________________________________________________________________________

using Pixels = vector<int>;

// class Observation
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


// Type definitions
using Observations = vector<Observation>;
using Distance = function <int(Pixels pixels1, Pixels pixels2)>;
using Classifier = function<string(Pixels pixels)>;

using namespace std::chrono;

//_________________________________________________________________________________________________

// observationData lambda
auto observationData = [](string& csvData) noexcept
{
  vector<string> columns;

  split(csvData, ',', columns);

  auto label = columns[0];
  Pixels pixels;

  for (size_t i = 1; i < columns.size(); ++i)
  {
    pixels.emplace_back(atoi(columns[i].c_str()));
  }

  return Observation(label, pixels);
};

//_________________________________________________________________________________________________
// reader lambda
auto reader = [](string path) noexcept
{
  auto allLines = readAllLines(path);

  allLines.erase(allLines.begin()); // Skip CSV header.

  Observations observations;

  //#define TRANSFORM

#ifdef TRANSFORM
  observations.resize(allLines.size());

  transform(allLines.begin(), allLines.end(), observations.begin(), observationData);
#else
  for (size_t line = 0; line < allLines.size(); ++line)
  {
    Observation observation = observationData(allLines[line]);
    observations.emplace_back(observation);
  }
#endif

  return observations;
};

//_________________________________________________________________________________________________

// main function
int main() noexcept
{
  // Start execution here. 
  // Some lambdas are define after this part, because they depend on definitions here.
  cout << "  Manhattan C++ Functional" << "\n";

  auto start = high_resolution_clock().now();

  auto trainingPath = R"(../Data/trainingsample.csv)";
  auto trainingData = reader(trainingPath);

  auto validationPath = R"(../Data/validationsample.csv)";
  auto validationData = reader(validationPath);

  //_________________________________________________________________________________________________

  // manhattanDistance lambda
  auto manhattanDistance = [](Pixels& pixels1, Pixels& pixels2)
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

  // classify function
  auto classify = [](Observations& trainingSet, Distance dist, Pixels& pixels) noexcept
  {
    auto shortest = DBL_MAX;
    auto currentBest = Observation();

    for (Observation& obs : trainingSet)
    {
      auto distance = dist(obs.pixels(), pixels);

      if (distance < shortest)
      {
        shortest = distance;
        currentBest = obs;
      }
    }

    return currentBest.label();
  };

  //_________________________________________________________________________________________________

  // manhattanClassifier lambda
  auto manhattanClassifier = [classify, &trainingData, manhattanDistance](Pixels& pixels) noexcept
  {
    return classify(trainingData, manhattanDistance, pixels);
  };

  //_________________________________________________________________________________________________

  // evaluate lambda
  auto evaluate = [](Observations& validationSet, Classifier classifier) noexcept
  {
    int sum = 0;
    auto count = validationSet.size();

    for (size_t i = 0; i < count; ++i)
    {
      auto obs = validationSet[i];

      if (classifier(obs.pixels()) == obs.label())
      {
        sum += 1;
      }
    }

    cout << "Correctly classified: " << ((double)sum) / count * 100.0 << " %\n";
  };

  //_________________________________________________________________________________________________

//  auto start = high_resolution_clock().now();
  
  evaluate(validationData, manhattanClassifier);  // Execute the digit recognizer process.

  auto finish = high_resolution_clock().now();
  auto elapsedMilliseconds = chrono::duration_cast<chrono::milliseconds>(finish - start);

  cout.precision(5);
  cout << "Elapsed time = " << elapsedMilliseconds.count() / 1000.0 << "s\n";

  cin.get();

  return 0;
}
