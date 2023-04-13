
from os import path

DEFAULT_COOKIE_PATH = path.join(path.dirname(__file__),'py_cookie')

open(DEFAULT_COOKIE_PATH,'w').write("hello")
