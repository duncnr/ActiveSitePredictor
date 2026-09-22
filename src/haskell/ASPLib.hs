{-# LANGUAGE ForeignFunctionInterface #-}
module ASPLib where

import Foreign.C.Types

haskell_add :: CInt -> CInt -> CInt
haskell_add x y = x + y
foreign export ccall haskell_add :: CInt -> CInt -> CInt
