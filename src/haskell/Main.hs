module Main where

import Data.Retrieval as DR

pipeline :: String -> String
pipeline e = DR.getPath e

main :: IO ()
main = putStrLn "hello world!"

