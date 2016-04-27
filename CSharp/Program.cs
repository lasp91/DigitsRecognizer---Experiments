using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;

namespace CSharp
{
  class Program
  {
    static void Main(string[] args)
    {
      Console.WriteLine("  Manhattan #");

      var start = DateTime.Now;

      var distance = new ManhattanDistance();
      var classifier = new BasicClassifier(distance);

      var trainingPath = @"..\..\..\Data\trainingsample.csv";
      var training = DataReader.ReadObservations(trainingPath);
      classifier.Train(training);

      var validationPath = @"..\..\..\Data\validationsample.csv";
      var validation = DataReader.ReadObservations(validationPath);

      var correct = Evaluator.Correct(validation, classifier);

      var finish = DateTime.Now;
      var elapsed = finish - start;

      Console.WriteLine("Correctly classified: {0:P2}", correct);
      Console.WriteLine($"Elapsed time = {elapsed.Seconds}sec {elapsed.Milliseconds}ms");
      Console.ReadLine();
    }
  }
}


