#!/bin/env python3
import requests
from os import path

DEFAULT_COOKIE_PATH = path.join(path.dirname(__file__),'py_cookie')

def save_cookie(cookie):
    return open(DEFAULT_COOKIE_PATH,'w').write(cookie)

def read_cookie():
    return open(DEFAULT_COOKIE_PATH,).read()


def login(url,username,password):
    r = None
    if path.exists(DEFAULT_COOKIE_PATH):
        r= requests.post(url,json={"username":username,"password":password},headers = { 'cookie' : read_cookie()})
    else:
        r= requests.post(url,json={"username":username,"password":password})
    res = r.json()
    print(res)
    cookie = r.headers['Set-cookie']
    print(cookie)
    if res['code'] == 0:
        save_cookie(cookie)
    return (res['code'],cookie)

if __name__ == "__main__":
    login('http://127.0.0.1:8099/user/login','test3','password')
