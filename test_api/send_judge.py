#!/bin/env python3
import requests
import json
from ojDef import baseUrl
from login import read_cookie
from os import path,system


json_path =  path.join( path.dirname(__file__),'send_judge.json' )
dataJson = json.loads( open(json_path).read() ) #.splitlines() 
# print(dataJson)

r = requests.post(baseUrl + 'handleJudge',json=dataJson,headers={'cookie':read_cookie()})
print(r.json())

slution_id = r.json()['msg']
# print(msg)

# 请求 评测的结果
r = requests.get(baseUrl + 'judge_result?id=' + slution_id,headers={'cookie':read_cookie()})
print(r.json())

# ws_url = ['wscat','-c','ws://127.0.0.1:8099/judgews?id=' + slution_id]

ws_url = 'wscat -c ws://127.0.0.1:8099/judgews?id='+ slution_id
ws_url += ' --header "cookie:' + read_cookie() + '"'
 
system(ws_url)
