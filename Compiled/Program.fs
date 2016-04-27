﻿// Learn more about F# at http://fsharp.org
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

    let manhattanDistance (pixels1, pixels2) =
        Array.zip pixels1 pixels2
        |> Array.map (fun (x, y) -> abs (x - y))
        |> Array.sum

    let euclideanDistance (pixels1, pixels2) =
        Array.zip pixels1 pixels2
        |> Array.map (fun (x, y) -> pown (x - y) 2)
        |> Array.sum

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

    let manhattanClassifier = classify trainingData manhattanDistance
    let euclideanClassifier = classify trainingData euclideanDistance
//    let manhattanClassifier = train trainingData manhattanDistance
//    let euclideanClassifier = train trainingData euclideanDistance

    let evaluate validationSet classifier = 
        validationSet
        |> Array.averageBy (fun x -> if classifier x.Pixels = x.Label then 1. else 0.)
        |> printfn "Correct: %.3f"

    let start = DateTime.Now

    printfn "  Manhattan F#"
    evaluate validationData manhattanClassifier

    let finish = DateTime.Now
    let elapsed = finish - start

    printfn "Elapsed time = %Asec %Ams" elapsed.Seconds elapsed.Milliseconds

    printfn "Euclidean"
    //evaluate validationData euclideanClassifier

    System.Console.ReadLine() |> ignore

// Illustration: full distance function

//let d (X,Y) = 
//    Array.zip X Y 
//    |> Array.map (fun (x,y) -> pown (x-y) 2) 
//    |> Array.sum 
//    |> sqrt

    0 // return an integer exit code