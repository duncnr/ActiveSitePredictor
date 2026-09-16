module Data.Retrieval where

-- import Network.Wreq
-- import Control.Lens (view)
-- import Data.Aeson.Lens (key, _String)
-- import qualified Data.ByteString.Lazy.Char8 as BL

getPath :: String -> String
getPath e = "https://rcsb.org/rest/v1/core/entry/" ++ e

