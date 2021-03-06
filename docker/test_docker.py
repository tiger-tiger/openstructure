import ost
from ost.mol.alg import qsscoring

# load two biounits to compare
ent_full = ost.io.LoadPDB('3ia3', remote=True)
ent_1 = ent_full.Select('cname=A,D')
ent_2 = ent_full.Select('cname=B,C')
# get score
ost.PushVerbosityLevel(3)
try:
    qs_scorer = qsscoring.QSscorer(ent_1, ent_2)
    ost.LogScript('QSscore:', str(qs_scorer.global_score))
    ost.LogScript('Chain mapping used:', str(qs_scorer.chain_mapping))
except qsscoring.QSscoreError as ex:
    # default handling: report failure and set score to 0
    ost.LogError('QSscore failed:', str(ex))
    qs_score = 0

print("OST is working!")
