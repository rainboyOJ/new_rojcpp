#!/bin/env python3
import requests
import json
from ../ojDef import baseUrl
from ../login import read_cookie
from os import path


json_path =  path.join( path.dirname(__file__),'send_judge.json' )
dataJson = json.loads( open(json_path).read() ) #.splitlines() 
print(dataJson)

r = requests.post(baseUrl + 'handleJudge',json=dataJson,headers={'cookie':read_cookie()})
print(r.json())

slution_id = r.json()['msg']
# print(msg)

# 请求 评测的结果
r = requests.get(baseUrl + 'judge_result?id=' + slution_id)
print(r.json())

