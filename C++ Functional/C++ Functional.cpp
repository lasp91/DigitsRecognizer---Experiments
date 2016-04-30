#include "stdafx.h"
#include <chrono>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <ppl.h>
#include <cmath> 
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

//_________________________________________________________________________________________________

// observationData lambda
auto observationData = [](string& csvData) noexcept
{
  vector<string> columns;

  split(csvData, ',', columns);

  auto label = columns[0];
  Pixels pixels;

  for (auto it = columns.begin(); it != columns.end(); ++it)
  {
    pixels.emplace_back(atoi(it->c_str()));
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
  // Some lambdas are defined after this part, because they depend on definitions here.
  cout << "  Manhattan C++ Functional" << "\n";

  using namespace std::chrono;

  auto start = high_resolution_clock().now();

  auto trainingPath = R"(../Data/trainingsample.csv)";
  auto trainingData = reader(trainingPath);

  auto validationPath = R"(../Data/validationsample.csv)";
  auto validationData = reader(validationPath);

  auto finish = high_resolution_clock().now();
  auto elapsedMilliseconds = chrono::duration_cast<chrono::milliseconds>(finish - start);

  cout.precision(5);
  cout << "Elapsed time for reading files = " << elapsedMilliseconds.count() / 1000.0 << "sec\n\n";
  
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
  
  // euclideanDistance lambda
  auto euclideanDistance = [](Pixels& pixels1, Pixels& pixels2)
  {
    if (pixels1.size() != pixels2.size())
    {
      throw std::length_error("Inconsistent image sizes.");
    }

    auto length = pixels1.size();
    auto distance = 0;

//#define USE_CONCURRENCY

#ifdef USE_CONCURRENCY
    concurrency::parallel_for(size_t(0), length, [&](size_t i)
    {
      // Use int product instead of pow() for performance.
//      distance += pow(pixels1[i] - pixels2[i], 2);
      int dif = pixels1[i] - pixels2[i];
      distance += dif * dif;
  });
#else
    for (size_t i = 0; i < length; i++)
    {
      // Use int product instead of pow() for performance.
//      distance += pow(pixels1[i] - pixels2[i], 2);
      int dif = pixels1[i] - pixels2[i];
      distance += dif * dif;
    }
#endif

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

  // euclideanClassifier lambda
  auto euclideanClassifier = [classify, &trainingData, euclideanDistance](Pixels& pixels) noexcept
  {
    return classify(trainingData, euclideanDistance, pixels);
  };

  //_________________________________________________________________________________________________

  // evaluate lambda
  auto evaluate = [](Observations& validationSet, Classifier classifier) noexcept
  {
    int sum = 0;

    for (auto it = validationSet.begin(); it != validationSet.end(); ++it)
    {
      if (classifier(it->pixels()) == it->label())
      {
        sum += 1;
      }
    }

    cout << "Correctly classified: " << ((double)sum) / validationSet.size() * 100.0 << " %\n";
  };

  //_________________________________________________________________________________________________

  // Use the Manhattan Distance
  {
    auto start = high_resolution_clock().now();

    evaluate(validationData, manhattanClassifier);  // Execute the digit recognizer process.

    auto finish = high_resolution_clock().now();
    auto elapsedMilliseconds = chrono::duration_cast<chrono::milliseconds>(finish - start);

    cout.precision(5);
    cout << "Elapsed time for Manhattan Distance = " << elapsedMilliseconds.count() / 1000.0 << "sec\n\n";
  }

  // Use the Euclidean Distance
  {
    auto start = high_resolution_clock().now();

    evaluate(validationData, euclideanClassifier);  // Execute the digit recognizer process.

    auto finish = high_resolution_clock().now();
    auto elapsedMilliseconds2 = chrono::duration_cast<chrono::milliseconds>(finish - start);

    cout.precision(5);
    cout << "Elapsed time for Euclidean Distance= " << elapsedMilliseconds2.count() / 1000.0 << "sec\n\n";
  }

  cout << "Press any key to terminate the program...";
  cin.get();

  return 0;
}
