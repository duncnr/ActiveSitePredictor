module Main where

import Data.Retrieval as R
import Data.Aeson
import qualified Data.ByteString.Lazy as BS

pipeline :: String -> IO BS.ByteString
pipeline e = R.pipe e

main :: IO ()
main = do
   putStrLn "Enter RCSB protein ID: "
   id <- getLine
   f <- pipeline id
   BS.writeFile ("./Test/" ++ id ++ ".txt") f
