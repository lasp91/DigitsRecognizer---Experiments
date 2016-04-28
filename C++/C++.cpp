// C++.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <chrono>
#include <vector>
#include <iostream>
#include <thread>
#include <stdexcept>

#include "CsvParser/include/csvparser.h"

using namespace std;
using namespace std::this_thread; // sleep_for, sleep_until
//using namespace std::chrono_literals; // ns, us, ms, s, h, etc.
//using std::chrono::system_clock;

// Observation
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

// Data Reader
using Observations = vector<Observation>;

class DataReader
{
  public: static Observations ReadObservations(string dataPath)
  {
    CsvParser* csvparser = CsvParser_new(dataPath.c_str(), ",", 1);
    CsvRow* header = nullptr;
    CsvRow* row = nullptr;
    int i = 0;
    Observations observations;

    header = CsvParser_getHeader(csvparser);
    const int count = CsvParser_getNumFields(header);

    while (row = CsvParser_getRow(csvparser))
    {
      const char** rowFields = CsvParser_getFields(row);
      auto label = rowFields[0];
      Pixels pixels;

      for (i = 1; i < count; i++)
      {
        pixels.emplace_back(atoi(rowFields[i]));
      }

      observations.emplace_back(Observation(label, pixels));
      CsvParser_destroy_row(row);
    }

    CsvParser_destroy(csvparser);

    return observations;
  }
};
  
// Distance
struct IDistance
{
  virtual double Between(Pixels pixels1, Pixels pixels2) = 0;
};

class ManhattanDistance : IDistance
{
public:
  double Between(Pixels pixels1, Pixels pixels2)
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
};

// Classifier
struct IClassifier
{
  virtual void Train(Observations trainingSet) = 0;
  virtual string Predict(Pixels pixels) = 0;
};

class BasicClassifier : IClassifier
{
public:
  BasicClassifier(IDistance* distance)  noexcept
  {
    _distance = distance;
  }

  void Train(Observations trainingSet)  noexcept
  {
    _data = trainingSet;
  }

  string Predict(Pixels pixels) noexcept
  {
    auto shortest = DBL_MAX;
    unique_ptr<Observation> currentBest(new Observation());

    for (Observation& obs : _data)
    {
      auto dist = _distance->Between(obs.pixels(), pixels);

      if (dist < shortest)
      {
        shortest = dist;
        *currentBest = obs;
      }
    }

    return currentBest->label();
  }

private:
  Observations _data;
  IDistance* _distance;
};

// Evaluator
class Evaluator
{
public:
  static double Correct(Observations validationSet, BasicClassifier* classifier) noexcept
  {
    double sum = 0.0;
    int count = 0;

    for (const Observation& obs : validationSet)
    {
      sum += Score(obs, (IClassifier*)classifier);
      count += 1;
    }

    return sum / count;
  }

private:
  static double Score(Observation obs, IClassifier* classifier) noexcept
  {
    if (classifier->Predict(obs.pixels()) == obs.label())
      return 1.0;
    else
      return 0.0;
  }
};

// Main program
using namespace std::chrono;

int main() noexcept
{
  cout << "  Manhattan C++" << "\n";

  auto start = high_resolution_clock().now();

  string trainingPath   = "../Data/trainingsample.csv";
  string validationPath = "../Data/validationsample.csv";

  auto training   = DataReader::ReadObservations(trainingPath);
  auto validation = DataReader::ReadObservations(validationPath);
  auto distance   = ManhattanDistance();
  auto classifier = BasicClassifier((IDistance*)&distance);

  classifier.Train(training);

  auto correct = Evaluator::Correct(validation, &classifier);
  auto finish = high_resolution_clock().now();

  cout << "Correctly classified: " << correct * 100.0 << " %\n";

  auto elapsedMilliseconds = chrono::duration_cast<chrono::milliseconds>(finish - start);
  
  cout.precision(5);
  cout << "Elapsed time = "<< elapsedMilliseconds.count() / 1000.0 << "s\n";

  cin.get();

  return 0;
}


//#include <fstream>

//ifstream file(path);
//auto good = file.good();


//  sleep_for(5s);

//  auto elapsedSeconds = chrono::duration<float>(finish - start);
//  printf("Elapsed time = %ll", elapsed.count());

//vector<string> values;

//while (in.getline(thisVal, MAXSIZE, ','))
//{
//  string str(thisVal);
//  values.push_back(std::move(str));
//  //std::move signals that I won't be using str anymore and that the underlying buffer can just be passed through to the new element. 
//  // After this str will not be valid. You can still assign to it normally but until you do any result of calling a method on it will be implementation defined.
//}
//or

//while (in.getline(thisVal, MAXSIZE, ','))
//{
//  values.emplace_back(thisVal);
//}

