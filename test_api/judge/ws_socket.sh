#!/bin/env bash
# need wscat
# install: yarn global add wscat
if [ x$1 != x ]
then
    wscat -c ws://127.0.0.1:8099/judgews?id=$1 --header "cookie:CSESSIONID=12E66ED6209B7807B06BB574F6E5B98A; path=/; expires=Fri, 06 May 2022 18:35:54 GMT"
else
    echo "需要一个参数,指明需要的得到结果的 solution_id"
fi
