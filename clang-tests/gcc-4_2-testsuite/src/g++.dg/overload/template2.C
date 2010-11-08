// { dg-options "-Wno-unused-value" }

typedef int PyObject;
typedef PyObject *(*PyCFunction)(PyObject *, PyObject *);
template<class T> int _clear(PyObject* self);	// { dg-error "note" }

void _typeInfo() 
{
  reinterpret_cast<PyCFunction>(_clear); // { dg-error "reinterpret_cast cannot resolve overloaded function" }
}
