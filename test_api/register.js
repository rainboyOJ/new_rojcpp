const axios = require('axios')

axios({
  method:'post',
  url:'http://127.0.0.1:8099/user/register',
  data: {username: 'root', nickname: 'root', password: 'password', email: 'root'}
}).then( function(res){
  console.log("res----------")
  console.log(res.data)
}).catch(function(err){
  console.log("err----------")
  console.log(err)
})
