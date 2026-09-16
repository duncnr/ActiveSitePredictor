{-# LANGUAGE ForeignFunctionInterface #-}

module Main where

import Data.Retrieval as DR

import Foreign.C.Types

foreign export ccall "haskell_add" add :: CInt -> CInt -> CInt

add :: CInt -> CInt -> CInt
add x y = x + y
{-# NOINLINE add #-}

pipeline :: String -> String
pipeline e = DR.getPath e

main :: IO ()
main = putStrLn "hello world!"
