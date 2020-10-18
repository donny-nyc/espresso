from distutils.sysconfig import get_python_inc
import platform
import os
import subprocess

DIR_OF_THIS_SCRIPT = os.path.abspath( os.path.dirname( __file__ ) )

flags = [
        '-Wall',
        '-Wextra',
        '-Werror',
        '-x', 'c',
        '-isystem', '/usr/include',
        '-isystem', '/usr/local/include',
        '-isystem', '/usr/include/sys',
]

def settings( **kwargs):
    return {
            'flags': flags
    }

