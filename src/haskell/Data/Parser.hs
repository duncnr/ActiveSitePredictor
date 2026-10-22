{-# LANGUAGE OverloadedStrings #-}

module Data.Parser 
   (AtomSite(..), Atom(..), AminoAcid(..), parser_pipe)
   where

------------------------------------------
-- TODO: find column indicies by header --
------------------------------------------

import qualified Data.ByteString.Lazy.Char8 as BSlc
import Data.Char
import Text.Read (readMaybe)

data AtomSite = 
   AtomSite {
      atom_id :: Int,
      atom_type :: Atom,
      x_coord :: Double,
      y_coord :: Double,
      z_coord :: Double,
      aa_seq_id :: Int,
      aa_type :: AminoAcid }
   deriving 
      (Show)

newtype Error = Error String
   deriving
      (Show)

data CIF_Token = 
   ArtifactID Int | Coord Double | CIF_Atom Atom | CIF_AminoAcid AminoAcid
   deriving
      (Show)

data Atom =
   Ca | Fl | Ni | Ox | Su
   deriving
      (Show)

data AminoAcid = 
   A | R | N | D | C | E | Q | G | H | I | L | K | M | F | P | S | T | W | Y | V
   deriving
      (Show, Eq)

-- pipeline function 
-- parserPipe :: BSlc.ByteString -> [Either Error Atom_Site]
parser_pipe :: BSlc.ByteString -> [AtomSite]
parser_pipe = check_structure . get_structure . 
   check_parse_lines . parse_lines . break_lines . trim_cif

-- break lines of cif file
--    atom spatial arrangement lines take the form 
--    ATOM/HETATM A_ID A_TYPE ... AA_TYPE AA_ID ... COORDS (X,Y,Z) ...
trim_cif :: BSlc.ByteString -> [BSlc.ByteString]
trim_cif = get_atom_table . BSlc.lines

get_atom_table :: [BSlc.ByteString] -> [BSlc.ByteString]
get_atom_table = takeWhile pred2 . dropWhile pred1   
   where
      pred1 = (/= "_atom_site.pdbx_PDB_model_num") . 
         BSlc.dropWhileEnd isSpace .
         BSlc.dropWhile isSpace 

      pred2 = (/= "#") . 
         BSlc.dropWhileEnd isSpace . 
         BSlc.dropWhile isSpace
      
-- break lines into consumable chunks
break_lines :: [BSlc.ByteString] -> [[(Int, BSlc.ByteString)]]
break_lines = map break_entry

break_entry :: BSlc.ByteString -> [(Int, BSlc.ByteString)]
break_entry e = clean_entry [ y | (x,y) <- zip [1..] (BSlc.words e), 
   x `elem` [1,2,3,6,9,11,12,13] ]

clean_entry :: [BSlc.ByteString] -> [(Int, BSlc.ByteString)]
clean_entry = zip [0..] 

-- itemize/classify tokens 
check_parse_lines :: [[Either Error CIF_Token]] -> [[CIF_Token]]
check_parse_lines = map check_inner_parse
   where
      check_inner_parse :: [Either Error CIF_Token] -> [CIF_Token]
      check_inner_parse (x:s) = case x of 
         Left (Error x) -> error ("Error: " ++ x)
         Right t -> t : check_inner_parse s
      check_inner_parse [] = []

parse_lines :: [[(Int, BSlc.ByteString)]] -> [[Either Error CIF_Token]]
parse_lines (o:os) = unpack_line o : parse_lines os
parse_lines [] = []

unpack_line :: [(Int, BSlc.ByteString)] -> [Either Error CIF_Token]
unpack_line ((i,e):es) 
   | BSlc.unpack e == "HETATM" = []
   | i == 0 = unpack_line es
   | otherwise =  lex_tokens i (BSlc.unpack e) : unpack_line es
unpack_line [] = []

lex_tokens :: Int -> String -> Either Error CIF_Token
lex_tokens i d 
   | i == 2 = case to_atom d of 
      Left x -> Left x
      Right a -> Right (CIF_Atom a)
   | i == 3 = case to_amino_acid d of
      Left x -> Left x
      Right aa -> Right (CIF_AminoAcid aa)
   | i `elem` [1,4] = case to_int d of
      Left x -> Left x
      Right id -> Right (ArtifactID id)
   | i `elem` [5..7] = case to_coord d of
      Left x -> Left x
      Right c -> Right (Coord c)
   | otherwise = Left (Error "unkown or invalid token")

to_int :: String -> Either Error Int
to_int s = case readMaybe s of
   Nothing -> Left (Error "cannot convert string to ID")
   Just x -> Right x

to_coord :: String -> Either Error Double
to_coord s = case readMaybe s of
   Nothing -> Left (Error "cannot convert string to Coordinate")
   Just x -> Right x

to_atom :: String -> Either Error Atom
to_atom a 
   | a == "C" = Right Ca
   | a == "F" = Right Fl
   | a == "N" = Right Ni
   | a == "O" = Right Ox
   | a == "S" = Right Su
   | otherwise = Left (Error "unknown or invalid atom")

to_amino_acid :: String -> Either Error AminoAcid
to_amino_acid aa
   | aa == "ALA" = Right A
   | aa == "ARG" = Right R
   | aa == "ASP" = Right D
   | aa == "ASN" = Right N
   | aa == "CYS" = Right C
   | aa == "GLU" = Right E
   | aa == "GLN" = Right Q
   | aa == "GLY" = Right G
   | aa == "HIS" = Right H
   | aa == "ILE" = Right I
   | aa == "LUE" = Right L
   | aa == "LYS" = Right K
   | aa == "MET" = Right M
   | aa == "PHE" = Right F
   | aa == "PRO" = Right P
   | aa == "SER" = Right S
   | aa == "THY" = Right T
   | aa == "TRP" = Right W
   | aa == "TYR" = Right Y
   | aa == "VAL" = Right V
   | otherwise = Left (Error "unknown or invalid amino acid")

-- use tokens to construct atomsites
check_structure :: [Either Error AtomSite] -> [AtomSite]
check_structure (as:ass) = case as of
   Left (Error x) -> error ("ERROR: " ++ x)
   Right s -> s : check_structure ass
check_structure [] = []

get_structure :: [[CIF_Token]] -> [Either Error AtomSite]
get_structure (t:ts) 
   | null t = get_structure ts
   | otherwise = construct_atom_site t : get_structure ts
get_structure [] = []

construct_atom_site :: [CIF_Token] -> Either Error AtomSite
construct_atom_site 
   (ArtifactID (a_id):
   CIF_Atom (a_t):
   CIF_AminoAcid (aa_t):
   ArtifactID (aa_id):
   Coord (x_c):
   Coord (y_c):
   Coord (z_c):[]) = Right (AtomSite {
      atom_id = a_id,
      atom_type = a_t,
      x_coord = x_c,
      y_coord = y_c,
      z_coord = z_c,
      aa_seq_id = aa_id,
      aa_type = aa_t})
construct_atom_site _ = Left (Error "not enough or too many artifacts to parse")

