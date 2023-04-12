## Intro

rewirte [CppDB: CppDB - SQL Connectivity Library](http://cppcms.com/sql/cppdb/index.html) use cpp++20( use Literals class as Template Parameters)



```plaintext
.
├── include
│   ├── backend.h
│   └── errors.hpp
├── src
│   └── backend.cpp
├── drivers
│   └── mysql_backend.cpp
└── tests
```


## 代码架构

```
query<static_str,Result<>>
       │
       │                         ┌───────────┐
       │                         │           │
       │                         │           │
       │       exec              │   pool    │
       └────────────────────────►│           │
                                 │           │
                                 └──┬────▲───┘
                                    │    │
                                    │    │
                                    │    │
                            shared_ptr   │
                                    │    │
                                    │  weak_ptr
                                    │    │                           ┌──────────────────────┐
                                    │    │                           │                      │
                                    ▼    │                           │ backend::connection  │
                                 connection_raii ───────────────────►│                      │
                                                    encapsulate      │                      │
                                                                     │ backend::result      │
                                                                     │                      │
                                                                     └──────────────────────┘
```


column->row(递归类)->schema(结果集)

query<command_str,schema>
