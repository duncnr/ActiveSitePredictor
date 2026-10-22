-- {-# LANGUAGE DataKinds #-}
{-# LANGUAGE OverloadedStrings #-}

module Main where

import Data.Retrieval
import Data.Parser
import Data.Aggregate
import Data.Char
import qualified Data.ByteString.Lazy as BSl
import qualified Data.ByteString.Lazy.Char8 as BSlc

-- pipeline :: String -> [Either Error Atom_Site]
pipeline :: String -> IO [AminoAcidSite]
pipeline id = do 
   rd <- retrieval_pipe id
   return (aggregate_pipe $ parser_pipe rd)

main :: IO ()
main = do
   putStrLn "Enter RCSB protein ID: "
   id <- getLine
   -- f <- pipeline $ map toLower id
   let p = map toLower id
   -- BSl.writeFile ("./Test/" ++ p ++ ".cif") f
   -- f <- BSl.readFile ("./Test/" ++ p ++ ".cif") 

   -- fileList <- map BSlc.unpack $ BSlc.lines f
   -- let fileList = BSlc.lines f

   -- print $ filter
   --    (BSlc.isPrefixOf "_atom_site.") fileList

   putStrLn "First 10: "
   as <- pipeline p
   mapM_ print $ (take 10 $ as)

