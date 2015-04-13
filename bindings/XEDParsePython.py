from ctypes import *

XEDPARSE_MAXBUFSIZE = 256
XEDPARSE_MAXASMSIZE = 16

XEDPARSE_ERROR = 0
XEDPARSE_OK = 1

class XEDPARSE(Structure):
    _pack_= 8
    _fields_ = [("x64", c_bool),
                ("cip", c_ulonglong),
                ("dest_size", c_uint),
                ("cbUnknown", c_void_p),
                ("dest", c_char * XEDPARSE_MAXASMSIZE),
                ("instr", c_char * XEDPARSE_MAXBUFSIZE),
                ("error", c_char * XEDPARSE_MAXBUFSIZE)
    ]

import os
if os.name == 'nt':
    __module = CDLL('XEDParse')
elif os.name == 'posix':
    __module = CDLL('libXEDParse.so')
    
XEDParseAssemble = __module.XEDParseAssemble