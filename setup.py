
from distutils.core import setup
from distutils.extension import Extension

# command line 
#$$> CFLAGS="-std=c++11" python ./setup.py build_ext --include-dirs="/home/andy/cpp/projects/quan-trunk"

# the c++ extension module
extension_mod = Extension('_joystick', sources = ['joystick.cpp'])

setup( name = "joystick", 
       ext_modules = [extension_mod])