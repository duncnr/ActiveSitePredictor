{-# LANGUAGE DataKinds #-}
{-# LANGUAGE OverloadedStrings #-}

module Data.Retrieval (pipe) where

import Network.HTTP.Req
import qualified Data.Text as T
import qualified Data.ByteString.Lazy as BS

pipe :: String -> IO BS.ByteString
pipe = get_data . get_RCSB_path 

get_RCSB_path :: String -> Url Https
get_RCSB_path eID = https 
   "files.rcsb.org" /: 
   "pub" /: 
   "pdb" /: 
   "data" /:
   "structures" /: 
   "all" /:
   "mmCIF" /:
   T.append (T.pack eID) ".cif.gz"

get_data :: Url Https -> IO BS.ByteString
get_data path = runReq defaultHttpConfig $ do 
   r <- req GET path NoReqBody lbsResponse mempty
   pure (responseBody r)

