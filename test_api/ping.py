#!/bin/env python3
import requests
from ojDef import baseUrl
from login import read_cookie

if __name__ == "__main__":
    print(read_cookie())
    r = requests.get(baseUrl + 'user/ping',headers= { 'cookie' : read_cookie()})
    print(r.json())


