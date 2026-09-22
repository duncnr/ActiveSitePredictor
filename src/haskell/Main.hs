module Main where

import Data.Retrieval as R
import Data.Char as C
import Foreign.C.Types
-- import Data.Aeson
import qualified Data.ByteString.Lazy as BS

pipeline :: String -> IO BS.ByteString
pipeline e = R.pipe e

main :: IO ()
main = do
   putStrLn "Enter RCSB protein ID: "
   id <- getLine
   f <- pipeline $ map C.toLower id
   BS.writeFile ("./Test/" ++ id ++ ".cif.gz") f

