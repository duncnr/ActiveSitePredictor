{-# LANGUAGE ForeignFunctionInterface #-}

module ASPLib where

import Foreign.C.Types

-- cpp calls
foreign export ccall haskell_add :: CInt -> CInt -> CInt

haskell_add :: CInt -> CInt -> CInt
haskell_add x y = x + y

