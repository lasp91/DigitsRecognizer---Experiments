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

class funcArray
{
public:
  funcArray(int size)
  {
    _funcArray = new int[size];
    _size = size;
  }

  ~funcArray()
  {
    delete _funcArray;
  }

  int* map(std::function<int(int)> f)
  {
    int* temp = new int[_size];

    for (auto i = 0; i < _size; i++)
    {
      temp[i] = f(i);
    }

    return temp;
  }

private:
  int* _funcArray;
  int _size;
};

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
        output.emplace_back(finalString);
      }
    }
  }

  return output.size();
}

auto readAllLines(string& path)
{
  ifstream file(path.c_str());

  if (!file)
  {
    // Any 'throw' type is good for the moment...
    throw std::invalid_argument(" **** File does not exist");
  }

  stringstream buffer;
  buffer << file.rdbuf();
  file.close();

  string str = buffer.str();
  vector<string> lines;

  split(str, '\n', lines);

  return lines;
}


// Type definitions
using Observations = vector<Observation>;
using Distance = function <int(Pixels pixels1, Pixels pixels2)>;
using Classifier = function<string(Pixels pixels)>;

auto observationData(string& csvData) noexcept
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
}

auto reader(string path) noexcept
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
}

auto manhattanDistance(Pixels& pixels1, Pixels& pixels2)
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
}

auto euclideanDistance(Pixels& pixels1, Pixels& pixels2)
{
  if (pixels1.size() != pixels2.size())
  {
    throw std::length_error("Inconsistent image sizes.");
  }

  auto length = pixels1.size();
  auto distance = 0;

  for (size_t i = 0; i < length; i++)
  {
    // Use int product instead of pow() for performance.
    //      distance += pow(pixels1[i] - pixels2[i], 2);
    int dif = pixels1[i] - pixels2[i];
    distance += dif * dif;
  }

  return distance;
}

auto classify(Observations& trainingData, Distance dist, Pixels& pixels) noexcept
{
  auto shortest = DBL_MAX;
  auto currentBest = Observation();

  for (Observation& obs : trainingData)
  {
    auto distance = dist(obs.pixels(), pixels);

    if (distance < shortest)
    {
      shortest = distance;
      currentBest = obs;
    }
  }

  return currentBest.label();
}

//auto manhattanClassifier(Observations& trainingData, Pixels& pixels) noexcept
//{
//  return classify(trainingData, manhattanDistance, pixels);
//}

// auto euclideanClassifier(Observations& trainingData, Pixels& pixels) noexcept
// {
//   return classify(trainingData, euclideanDistance, pixels);
// }

#define USE_CONCURRENCY

auto evaluate(Observations& validationData, Classifier classifier) noexcept
{
  auto length = validationData.size();

#ifdef USE_CONCURRENCY
#include <atomic>

  atomic<int> sum = 0;

  concurrency::parallel_for_each(begin(validationData), end(validationData), [&](Observation& it)
  {
    if (classifier(it.pixels()) == it.label())
    {
      sum += 1;
    }
  });
#else
  int sum = 0;

  for (auto it = begin(validationSet); it != end(validationSet); ++it)
  {
    if (classifier(it->pixels()) == it->label())
    {
      sum += 1;
    }
  }
#endif
  cout << "Sum = " << sum << "\n";  // For testing purpose only.
  cout << "Correctly classified: " << ((double)sum) / length * 100.0 << " %\n";
}

auto evaluateAndPrintElapsedTime(Observations& validationData, Classifier classifier, string message)
{
  using namespace std::chrono;

  auto start = high_resolution_clock().now();

  evaluate(validationData, classifier);  // Execute the digit recognizer process.

  auto finish = high_resolution_clock().now();
  auto elapsedMilliseconds = chrono::duration_cast<chrono::milliseconds>(finish - start);

  cout.precision(5);
  cout << message << elapsedMilliseconds.count() / 1000.0 << "sec\n\n";
}

// main function
int main() noexcept
{
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
  
  // manhattanClassifier lambda
  auto manhattanClassifier = [&trainingData](Pixels& pixels) noexcept
  {
    return classify(trainingData, manhattanDistance, pixels);
  };

  // euclideanClassifier lambda
  auto euclideanClassifier = [&trainingData](Pixels& pixels) noexcept
  {
    return classify(trainingData, euclideanDistance, pixels);
  };

  // Use the Manhattan Distance
  evaluateAndPrintElapsedTime(validationData, manhattanClassifier, "Elapsed time for Manhattan Distance = ");  // Execute the digit recognizer process.

  // Use the Euclidean Distance
  evaluateAndPrintElapsedTime(validationData, euclideanClassifier, "Elapsed time for Euclidean Distance = ");  // Execute the digit recognizer process.

  cout << "Press any key to terminate the program...";
  cin.get();

  return 0;
}
