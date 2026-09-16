module Main where

import Data.Retrieval as DR

pipeline :: String -> String
pipeline e = DR.pipe e

main :: IO ()
main = do
   putStrLn "Enter RCSB protein ID: "
   id <- getLine
   putStrLn (pipeline id)

