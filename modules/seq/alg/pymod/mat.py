from ost.seq.alg import SubstWeightMatrix

def _InitMatrix(data):
  """
  Initialise a new substitution weight matrix with the weights in data. data
  must be a 24x24 array of ints.
  """
  mat=SubstWeightMatrix()
  # string of valid amino acid one letter codes
  chars='ABCDEFGHIKLMNPQRSTVWXYZ'
  for i, row in enumerate(data):
    for j, weight in enumerate(row):
      mat.SetWeight(chars[i], chars[j], weight)
  return mat

_RAW_BLOSUM45_DATA=(
  ( 5, -1, -1, -2, -1, -2,  0, -2, -1, -1, -1, -1, -1, -1, -1, -2,  1,  0,  0, -2,  0, -2, -1),
  (-1,  4, -2,  5,  1, -3, -1,  0, -3,  0, -3, -2,  4, -2,  0, -1,  0,  0, -3, -4, -1, -2,  2),
  (-1, -2, 12, -3, -3, -2, -3, -3, -3, -3, -2, -2, -2, -4, -3, -3, -1, -1, -1, -5, -2, -3, -3),
  (-2,  5, -3,  7,  2, -4, -1,  0, -4,  0, -3, -3,  2, -1,  0, -1,  0, -1, -3, -4, -1, -2,  1),
  (-1,  1, -3,  2,  6, -3, -2,  0, -3,  1, -2, -2,  0,  0,  2,  0,  0, -1, -3, -3, -1, -2,  4),
  (-2, -3, -2, -4, -3,  8, -3, -2,  0, -3,  1,  0, -2, -3, -4, -2, -2, -1,  0,  1, -1,  3, -3),
  ( 0, -1, -3, -1, -2, -3,  7, -2, -4, -2, -3, -2,  0, -2, -2, -2,  0, -2, -3, -2, -1, -3, -2),
  (-2,  0, -3,  0,  0, -2, -2, 10, -3, -1, -2,  0,  1, -2,  1,  0, -1, -2, -3, -3, -1,  2,  0),
  (-1, -3, -3, -4, -3,  0, -4, -3,  5, -3,  2,  2, -2, -2, -2, -3, -2, -1,  3, -2, -1,  0, -3),
  (-1,  0, -3,  0,  1, -3, -2, -1, -3,  5, -3, -1,  0, -1,  1,  3, -1, -1, -2, -2, -1, -1,  1),
  (-1, -3, -2, -3, -2,  1, -3, -2,  2, -3,  5,  2, -3, -3, -2, -2, -3, -1,  1, -2, -1,  0, -2),
  (-1, -2, -2, -3, -2,  0, -2,  0,  2, -1,  2,  6, -2, -2,  0, -1, -2, -1,  1, -2, -1,  0, -1),
  (-1,  4, -2,  2,  0, -2,  0,  1, -2,  0, -3, -2,  6, -2,  0,  0,  1,  0, -3, -4, -1, -2,  0),
  (-1, -2, -4, -1,  0, -3, -2, -2, -2, -1, -3, -2, -2,  9, -1, -2, -1, -1, -3, -3, -1, -3, -1),
  (-1,  0, -3,  0,  2, -4, -2,  1, -2,  1, -2,  0,  0, -1,  6,  1,  0, -1, -3, -2, -1, -1,  4),
  (-2, -1, -3, -1,  0, -2, -2,  0, -3,  3, -2, -1,  0, -2,  1,  7, -1, -1, -2, -2, -1, -1,  0),
  ( 1,  0, -1,  0,  0, -2,  0, -1, -2, -1, -3, -2,  1, -1,  0, -1,  4,  2, -1, -4,  0, -2,  0),
  ( 0,  0, -1, -1, -1, -1, -2, -2, -1, -1, -1, -1,  0, -1, -1, -1,  2,  5,  0, -3,  0, -1, -1),
  ( 0, -3, -1, -3, -3,  0, -3, -3,  3, -2,  1,  1, -3, -3, -3, -2, -1,  0,  5, -3, -1, -1, -3),
  (-2, -4, -5, -4, -3,  1, -2, -3, -2, -2, -2, -2, -4, -3, -2, -2, -4, -3, -3, 15, -2,  3, -2),
  ( 0, -1, -2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  0,  0, -1, -2, -1, -1, -1),
  (-2, -2, -3, -2, -2,  3, -3,  2,  0, -1,  0,  0, -2, -3, -1, -1, -2, -1, -1,  3, -1,  8, -2),
  (-1,  2, -3,  1,  4, -3, -2,  0, -3,  1, -2, -1,  0, -1,  4,  0,  0, -1, -3, -2, -1, -2,  4),
)
      
_RAW_BLOSUM62_DATA=(
  ( 4, -2,  0, -2, -1, -2,  0, -2, -1, -1, -1, -1, -2, -1, -1, -1,  1,  0,  0, -3,  0, -2, -1),
  (-2,  4, -3,  4,  1, -3, -1,  0, -3,  0, -4, -3,  3, -2,  0, -1,  0, -1, -3, -4, -1, -3,  1),
  ( 0, -3,  9, -3, -4, -2, -3, -3, -1, -3, -1, -1, -3, -3, -3, -3, -1, -1, -1, -2, -2, -2, -3),
  (-2,  4, -3,  6,  2, -3, -1, -1, -3, -1, -4, -3,  1, -1,  0, -2,  0, -1, -3, -4, -1, -3,  1),
  (-1,  1, -4,  2,  5, -3, -2,  0, -3,  1, -3, -2,  0, -1,  2,  0,  0, -1, -2, -3, -1, -2,  4),
  (-2, -3, -2, -3, -3,  6, -3, -1,  0, -3,  0,  0, -3, -4, -3, -3, -2, -2, -1,  1, -1,  3, -3),
  ( 0, -1, -3, -1, -2, -3,  6, -2, -4, -2, -4, -3,  0, -2, -2, -2,  0, -2, -3, -2, -1, -3, -2),
  (-2,  0, -3, -1,  0, -1, -2,  8, -3, -1, -3, -2,  1, -2,  0,  0, -1, -2, -3, -2, -1,  2,  0),
  (-1, -3, -1, -3, -3,  0, -4, -3,  4, -3,  2,  1, -3, -3, -3, -3, -2, -1,  3, -3, -1, -1, -3),
  (-1,  0, -3, -1,  1, -3, -2, -1, -3,  5, -2, -1,  0, -1,  1,  2,  0, -1, -2, -3, -1, -2,  1),
  (-1, -4, -1, -4, -3,  0, -4, -3,  2, -2,  4,  2, -3, -3, -2, -2, -2, -1,  1, -2, -1, -1, -3),
  (-1, -3, -1, -3, -2,  0, -3, -2,  1, -1,  2,  5, -2, -2,  0, -1, -1, -1,  1, -1, -1, -1, -1),
  (-2,  3, -3,  1,  0, -3,  0,  1, -3,  0, -3, -2,  6, -2,  0,  0,  1,  0, -3, -4, -1, -2,  0),
  (-1, -2, -3, -1, -1, -4, -2, -2, -3, -1, -3, -2, -2,  7, -1, -2, -1, -1, -2, -4, -2, -3, -1),
  (-1,  0, -3,  0,  2, -3, -2,  0, -3,  1, -2,  0,  0, -1,  5,  1,  0, -1, -2, -2, -1, -1,  3),
  (-1, -1, -3, -2,  0, -3, -2,  0, -3,  2, -2, -1,  0, -2,  1,  5, -1, -1, -3, -3, -1, -2,  0),
  ( 1,  0, -1,  0,  0, -2,  0, -1, -2,  0, -2, -1,  1, -1,  0, -1,  4,  1, -2, -3,  0, -2,  0),
  ( 0, -1, -1, -1, -1, -2, -2, -2, -1, -1, -1, -1,  0, -1, -1, -1,  1,  5,  0, -2,  0, -2, -1),
  ( 0, -3, -1, -3, -2, -1, -3, -3,  3, -2,  1,  1, -3, -2, -2, -3, -2,  0,  4, -3, -1, -1, -2),
  (-3, -4, -2, -4, -3,  1, -2, -2, -3, -3, -2, -1, -4, -4, -2, -3, -3, -2, -3, 11, -2,  2, -3),
  ( 0, -1, -2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -2, -1, -1,  0,  0, -1, -2, -1, -1, -1),
  (-2, -3, -2, -3, -2,  3, -3,  2, -1, -2, -1, -1, -2, -3, -1, -2, -2, -2, -1,  2, -1,  7, -2),
  (-1,  1, -3,  1,  4, -3, -2,  0, -3,  1, -3, -1,  0, -1,  3,  0,  0, -1, -2, -3, -1, -2,  4),
)

_RAW_BLOSUM80_DATA=(
  ( 7, -3, -1, -3, -2, -4,  0, -3, -3, -1, -3, -2, -3, -1, -2, -3,  2,  0, -1, -5, -1, -4, -2),
  (-3,  6, -6,  6,  1, -6, -2, -1, -6, -1, -7, -5,  5, -4, -1, -2,  0, -1, -6, -8, -3, -5,  0),
  (-1, -6, 13, -7, -7, -4, -6, -7, -2, -6, -3, -3, -5, -6, -5, -6, -2, -2, -2, -5, -4, -5, -7),
  (-3,  6, -7, 10,  2, -6, -3, -2, -7, -2, -7, -6,  2, -3, -1, -3, -1, -2, -6, -8, -3, -6,  1),
  (-2,  1, -7,  2,  8, -6, -4,  0, -6,  1, -6, -4, -1, -2,  3, -1, -1, -2, -4, -6, -2, -5,  6),
  (-4, -6, -4, -6, -6, 10, -6, -2, -1, -5,  0,  0, -6, -6, -5, -5, -4, -4, -2,  0, -3,  4, -6),
  ( 0, -2, -6, -3, -4, -6,  9, -4, -7, -3, -7, -5, -1, -5, -4, -4, -1, -3, -6, -6, -3, -6, -4),
  (-3, -1, -7, -2,  0, -2, -4, 12, -6, -1, -5, -4,  1, -4,  1,  0, -2, -3, -5, -4, -2,  3,  0),
  (-3, -6, -2, -7, -6, -1, -7, -6,  7, -5,  2,  2, -6, -5, -5, -5, -4, -2,  4, -5, -2, -3, -6),
  (-1, -1, -6, -2,  1, -5, -3, -1, -5,  8, -4, -3,  0, -2,  2,  3, -1, -1, -4, -6, -2, -4,  1),
  (-3, -7, -3, -7, -6,  0, -7, -5,  2, -4,  6,  3, -6, -5, -4, -4, -4, -3,  1, -4, -2, -2, -5),
  (-2, -5, -3, -6, -4,  0, -5, -4,  2, -3,  3,  9, -4, -4, -1, -3, -3, -1,  1, -3, -2, -3, -3),
  (-3,  5, -5,  2, -1, -6, -1,  1, -6,  0, -6, -4,  9, -4,  0, -1,  1,  0, -5, -7, -2, -4, -1),
  (-1, -4, -6, -3, -2, -6, -5, -4, -5, -2, -5, -4, -4, 12, -3, -3, -2, -3, -4, -7, -3, -6, -2),
  (-2, -1, -5, -1,  3, -5, -4,  1, -5,  2, -4, -1,  0, -3,  9,  1, -1, -1, -4, -4, -2, -3,  5),
  (-3, -2, -6, -3, -1, -5, -4,  0, -5,  3, -4, -3, -1, -3,  1,  9, -2, -2, -4, -5, -2, -4,  0),
  ( 2,  0, -2, -1, -1, -4, -1, -2, -4, -1, -4, -3,  1, -2, -1, -2,  7,  2, -3, -6, -1, -3, -1),
  ( 0, -1, -2, -2, -2, -4, -3, -3, -2, -1, -3, -1,  0, -3, -1, -2,  2,  8,  0, -5, -1, -3, -2),
  (-1, -6, -2, -6, -4, -2, -6, -5,  4, -4,  1,  1, -5, -4, -4, -4, -3,  0,  7, -5, -2, -3, -4),
  (-5, -8, -5, -8, -6,  0, -6, -4, -5, -6, -4, -3, -7, -7, -4, -5, -6, -5, -5, 16, -5,  3, -5),
  (-1, -3, -4, -3, -2, -3, -3, -2, -2, -2, -2, -2, -2, -3, -2, -2, -1, -1, -2, -5, -2, -3, -1),
  (-4, -5, -5, -6, -5,  4, -6,  3, -3, -4, -2, -3, -4, -6, -3, -4, -3, -3, -3,  3, -3, 11, -4),
  (-2,  0, -7,  1,  6, -6, -4,  0, -6,  1, -5, -3, -1, -2,  5,  0, -1, -2, -4, -5, -1, -4,  6),
)

_RAW_BLOSUM100_DATA=(
  ( 8, -4, -2, -5, -3, -5, -1, -4, -4, -2, -4, -3, -4, -2, -2, -3,  1, -1, -2, -6, -2, -5, -2),
  (-4,  6, -7,  6,  0, -7, -3, -2, -8, -2, -8, -7,  5, -5, -2, -4, -1, -2, -7, -9, -4, -6,  0),
  (-2, -7, 14, -8, -9, -4, -7, -8, -3, -8, -5, -4, -5, -8, -7, -8, -3, -3, -3, -7, -5, -6, -8),
  (-5,  6, -8, 10,  2, -8, -4, -3, -8, -3, -8, -8,  1, -5, -2, -5, -2, -4, -8,-10, -4, -7,  0),
  (-3,  0, -9,  2, 10, -8, -6, -2, -7,  0, -7, -5, -2, -4,  2, -2, -2, -3, -5, -8, -3, -7,  7),
  (-5, -7, -4, -8, -8, 11, -8, -4, -2, -6,  0, -1, -7, -7, -6, -6, -5, -5, -3,  0, -4,  4, -7),
  (-1, -3, -7, -4, -6, -8,  9, -6, -9, -5, -8, -7, -2, -6, -5, -6, -2, -5, -8, -7, -4, -8, -5),
  (-4, -2, -8, -3, -2, -4, -6, 13, -7, -3, -6, -5,  0, -5,  1, -1, -3, -4, -7, -5, -4,  1, -1),
  (-4, -8, -3, -8, -7, -2, -9, -7,  8, -6,  2,  1, -7, -7, -6, -7, -5, -3,  4, -6, -3, -4, -7),
  (-2, -2, -8, -3,  0, -6, -5, -3, -6, 10, -6, -4, -1, -3,  2,  3, -2, -3, -5, -8, -3, -5,  0),
  (-4, -8, -5, -8, -7,  0, -8, -6,  2, -6,  8,  3, -7, -7, -5, -6, -6, -4,  0, -5, -3, -4, -6),
  (-3, -7, -4, -8, -5, -1, -7, -5,  1, -4,  3, 12, -5, -5, -2, -4, -4, -2,  0, -4, -3, -5, -4),
  (-4,  5, -5,  1, -2, -7, -2,  0, -7, -1, -7, -5, 11, -5, -1, -2,  0, -1, -7, -8, -3, -5, -2),
  (-2, -5, -8, -5, -4, -7, -6, -5, -7, -3, -7, -5, -5, 12, -4, -5, -3, -4, -6, -8, -4, -7, -4),
  (-2, -2, -7, -2,  2, -6, -5,  1, -6,  2, -5, -2, -1, -4, 11,  0, -2, -3, -5, -5, -2, -4,  5),
  (-3, -4, -8, -5, -2, -6, -6, -1, -7,  3, -6, -4, -2, -5,  0, 10, -3, -3, -6, -7, -3, -5, -1),
  ( 1, -1, -3, -2, -2, -5, -2, -3, -5, -2, -6, -4,  0, -3, -2, -3,  9,  2, -4, -7, -2, -5, -2),
  (-1, -2, -3, -4, -3, -5, -5, -4, -3, -3, -4, -2, -1, -4, -3, -3,  2,  9, -1, -7, -2, -5, -3),
  (-2, -7, -3, -8, -5, -3, -8, -7,  4, -5,  0,  0, -7, -6, -5, -6, -4, -1,  8, -5, -3, -5, -5),
  (-6, -9, -7, 10, -8,  0, -7, -5, -6, -8, -5, -4, -8, -8, -5, -7, -7, -7, -5, 17, -6,  2, -7),
  (-2, -4, -5, -4, -3, -4, -4, -4, -3, -3, -3, -3, -3, -4, -2, -3, -2, -2, -3, -6, -3, -4, -2),
  (-5, -6, -6, -7, -7,  4, -8,  1, -4, -5, -4, -5, -5, -7, -4, -5, -5, -5, -5,  2, -4, 12, -6),
  (-2,  0, -8,  0,  7, -7, -5, -1, -7,  0, -6, -4, -2, -4,  5, -1, -2, -3, -5, -7, -2, -6,  6),
)

BLOSUM45=_InitMatrix(_RAW_BLOSUM45_DATA)
BLOSUM62=_InitMatrix(_RAW_BLOSUM62_DATA)
BLOSUM80=_InitMatrix(_RAW_BLOSUM80_DATA)
BLOSUM100=_InitMatrix(_RAW_BLOSUM100_DATA)

__all__=['BLOSUM45','BLOSUM62','BLOSUM80','BLOSUM100']