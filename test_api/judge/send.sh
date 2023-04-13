#!/bin/env bash
SHELL_FOLDER=$(dirname "$0")
data_path="@"${SHELL_FOLDER}/send_judge.json
echo ${data_path}
curl -v -X POST http://127.0.0.1:8099/handleJudge \
    --cookie cookie \
    --cookie-jar cookie \
   -H 'content-type: application/json' \
   --data ${data_path}
