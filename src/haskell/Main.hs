{-# LANGUAGE ForeignFunctionInterface #-}

module Main where

import Data.Retrieval as R
import Data.Aeson
import Foreign.C.Types
import qualified Data.ByteString.Lazy as BS

{-
foreign export ccall "haskell_add" add :: CInt -> CInt -> CInt

add :: CInt -> CInt -> CInt
add x y = x + y
{-# NOINLINE add #-}
-}

pipeline :: String -> IO BS.ByteString
pipeline e = R.pipe e

main :: IO ()
main = do
   putStrLn "Enter RCSB protein ID: "
   id <- getLine
   f <- pipeline id
   BS.writeFile ("./Test/" ++ id ++ ".txt") f

