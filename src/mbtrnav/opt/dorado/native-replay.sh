# Very simple shell script that runs trn_replay in native mode.
# Afterwards the script copies the terrainAid.cfg from the source
# log directory to the new latest replay log directory.
# 
echo "Running trn_replay in native mode..."
echo "     linux/trn_replay $1 $2 $3 $4 $5 $6 $7 $8 -h native"
linux/trn_replay $1 $2 $3 $4 $5 $6 $7 $8 -h native && cp -pv $2/terrainAid.cfg $TRN_LOGFILES/latestTRN/terrainAid.cfg