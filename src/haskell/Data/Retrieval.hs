module Data.Retrieval (pipe) where

import Network.HTTP.Req
-- import Control.Lens (view)
-- import Data.Aeson.Lens (key, _String)
-- import qualified Data.ByteString.Lazy.Char8 as BL

pipe :: String -> Url
pipe = getPath e

getPath :: String -> Url 
getPath e = https "rcsb.org" /: "rest" /: "v1" /: "core" /: "entry" /: e



