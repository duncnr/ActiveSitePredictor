module Data.Aggregate 
   (AminoAcidSite(..), AAClassifier, aggregate_pipe)
   where

import Data.Parser (AtomSite(..), AminoAcid(..))

data AminoAcidSite = 
   AminoAcidSite {
      seq_id :: Int,
      aa :: AminoAcid,
      coords :: (Double, Double, Double),
      prop :: AAClassifier }
   deriving 
      (Show)

data AAClassifier =
   Nonpolar | Polar | Acidic | Basic
   deriving
      (Show)

aggregate_pipe :: [AtomSite] -> [AminoAcidSite]
aggregate_pipe = convert_atoms . group_atoms

-- break records based on amino acid seq
group_atoms :: [AtomSite] -> [[AtomSite]]
group_atoms (a:as) = takeWhile (\x -> (aa_seq_id x) == (aa_seq_id a)) as :
   (group_atoms $ dropWhile (\x -> (aa_seq_id x) == (aa_seq_id a)) as)
group_atoms [] = []

-- converts grouped atom sites into amino acids
convert_atoms :: [[AtomSite]] -> [AminoAcidSite]
convert_atoms = map (to_amino_acid_site)

-- aggregate sublist of atoms relating to a single amino acid with an
--    average coordinate and added amino acid property
to_amino_acid_site :: [AtomSite] -> AminoAcidSite
to_amino_acid_site (a:as) = AminoAcidSite {
   seq_id = aa_seq_id a,
   aa = aa_type a,
   coords = collect_coords (a:as),
   prop = classify_aa (aa_type a) }

collect_coords :: [AtomSite] -> (Double, Double, Double)
collect_coords as = (avg $ get_x as, avg $ get_y as, avg $ get_z as)
   where
      get_x (a:as) = x_coord a : get_x as
      get_x [] = []

      get_y (a:as) = y_coord a : get_y as
      get_y [] = []

      get_z (a:as) = z_coord a : get_z as
      get_z [] = []

      avg xs = sum xs / fromIntegral (length xs)

classify_aa :: AminoAcid -> AAClassifier
classify_aa x
   | x `elem` [A,V,L,I,M,F,W,P] = Nonpolar
   | x `elem` [S,T,C,Y,N,Q,G] = Polar
   | x `elem` [D,E] = Acidic
   | x `elem` [H,K,R] = Basic
   | otherwise = error ("ERROR: invalid amino acid - " ++ show x)

