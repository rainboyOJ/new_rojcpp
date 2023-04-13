curl -v -X POST http://127.0.0.1:8099/user/register \
   -H 'Content-Type: application/json' \
   -d '{"username": "test3" , "nickname": "test2_nickname" ,"password": "password", "school":   "fake school", "email":    "fake3@qq.mail"}'
