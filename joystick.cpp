
#include <cstdint>
#include <cstdio>
#include <quan/joystick.hpp>

#include <python2.7/Python.h>

namespace {

   quan::joystick * js = nullptr;
   
   PyObject * read_joystick(PyObject *self, PyObject *args)
   {
      if ( js != nullptr){
         if (js->is_running()){
            int32_t channel = -1;
            if (PyArg_ParseTuple(args, "i", &channel)){
               int32_t value = js->get_channel(channel);
               return Py_BuildValue("i",&value);
            }
         }
      }
      return nullptr;
   }

   PyMethodDef JoystickMethods[] = {
      { "read_joystick", read_joystick, METH_VARARGS, "Read Joystick channel N" },
      { NULL, NULL, 0, NULL } // sentinel
   };
}

PyMODINIT_FUNC initjoystick(void)
{
    js = new quan::joystick{"/dev/js0");
    Py_InitModule("read_joystick", JoystickMethods);
}

//int main()
//{
//
//}

