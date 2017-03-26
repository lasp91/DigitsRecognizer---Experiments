// Learn more about F# at http://fsharp.org
// See the 'F# Tutorial' project for more help.
open System
open System.IO

type Observation = { Label : string; Pixels : int[] }
type Distance = int[] * int[] -> int
type Classifier = int[] -> string

[<EntryPoint>]
let main argv = 
    printfn "%A" argv

    let ObservationData (csvData : string) =
        let columns = csvData.Split(',')
        let label = columns.[0]
        let pixels = columns.[1..] |> Array.map int 
        { Label = label; Pixels = pixels }

    let reader path = 
        let data = File.ReadAllLines path
        data.[1..]
        |> Array.map ObservationData

    let trainingPath = __SOURCE_DIRECTORY__ + @"../../Data/trainingsample.csv"
    let trainingData = reader trainingPath

    let validationPath = __SOURCE_DIRECTORY__ + @"../../Data/validationsample.csv"
    let validationData = reader validationPath

    let manhattanDistance (pixels1, pixels2) =  // Using zip and map
        Array.zip pixels1 pixels2
        |> Array.map (fun (x, y) -> abs (x - y))
        |> Array.sum

    let manhattanDistance3 (pixels1, pixels2) = // Using map2
        (pixels1, pixels2)
        ||> Array.map2 (fun x y -> abs (x - y))
        |> Array.sum

    let manhattanDistanceImperative (pixels1 : int[], pixels2 : int[]) =
        let mutable sum = 0

        for i = 0 to pixels1.Length - 1 do
          sum <- sum + abs(pixels1.[i] - pixels2.[i])
        sum

    let euclideanDistance (pixels1, pixels2) =
        Array.zip pixels1 pixels2
//        |> Array.map (fun (x, y) -> pown (x - y) 2)
        |> Array.map (fun (x, y) -> (x - y) * (x - y))
        |> Array.sum

    let euclideanDistanceImperative (pixels1 : int[], pixels2 : int[]) =
        let mutable sum = 0

        for i = 0 to pixels1.Length - 1 do
          let dif = abs(pixels1.[i] - pixels2.[i])
          sum <- sum + dif * dif
        sum

    let classify (trainingSet : Observation[]) (dist : Distance)  (pixels : int[]) =
        trainingSet
        |> Array.minBy (fun x -> dist (x.Pixels, pixels))
        |> fun x -> x.Label
        

//    let train (trainingSet : Observation[]) (dist : Distance) =
//        let classify (pixels : int[]) =
//            trainingSet
//            |> Array.minBy (fun x -> dist (x.Pixels, pixels))
//            |> fun x -> x.Label
//        classify

    let manhattanClassifier = classify trainingData manhattanDistanceImperative
    let euclideanClassifier = classify trainingData euclideanDistanceImperative
//    let manhattanClassifier = train trainingData manhattanDistance
//    let euclideanClassifier = train trainingData euclideanDistance

#if USE_CONCURRENCY // Use parallel map
    let evaluate validationSet classifier = 
        validationSet
        |> Array.Parallel.map (fun x -> if classifier x.Pixels = x.Label then 1. else 0.)
        |> Array.average
        |> printfn "Parallel - Correct: %.3f"
#else
    let evaluate validationSet classifier = 
        validationSet
        |> Array.averageBy (fun x -> if (classifier x.Pixels) = x.Label then 1. else 0.)
        |> printfn "Correct: %.3f"
#endif

    let start = DateTime.Now

    printfn "  Manhattan F#"
    evaluate validationData manhattanClassifier

    let finish = DateTime.Now
    let elapsed = finish - start

    printfn "Elapsed time = %Asec %Ams\n" elapsed.Seconds elapsed.Milliseconds

// Ignoring the Euclidean classifier for now... Simple test.
    let start = DateTime.Now

    printfn "  Euclidean F#"
    evaluate validationData euclideanClassifier

    let finish = DateTime.Now
    let elapsed = finish - start

    printfn "Elapsed time = %Asec %Ams" elapsed.Seconds elapsed.Milliseconds
    System.Console.ReadLine() |> ignore

// Redefining a variable doesn't seem to cause an error....
    let aa = 8
    printfn ">>>> %d" aa

    let aa = 9
    printfn ">>>> %d" aa

// Illustration: full distance function

//let d (X,Y) = 
//    Array.zip X Y 
//    |> Array.map (fun (x,y) -> pown (x-y) 2) 
//    |> Array.sum 
//    |> sqrt

    0 // return an integer exit code
