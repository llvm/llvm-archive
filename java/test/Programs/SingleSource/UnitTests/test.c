#include <stdio.h>

#include "Test.h"

void Java_Test_println__Z(JNIEnv *env, jclass clazz, jboolean aBoolean)
{
  if (aBoolean)
    puts("true");
  else
    puts("false");
}

void Java_Test_println__I(JNIEnv *env, jclass clazz, jint aInt)
{
  printf("%d\n", aInt);
}

void Java_Test_println__J(JNIEnv *env, jclass clazz, jlong aLong)
{
  printf("%Ld\n", aLong);
}

void Java_Test_println__F(JNIEnv *env, jclass clazz, jfloat aFloat)
{
  printf("%f\n", aFloat);
}

void Java_Test_println__D(JNIEnv *env, jclass clazz, jdouble aDouble)
{
  printf("%f\n", aDouble);
}

void Java_Test_println___3B(JNIEnv *env, jclass clazz, jbyteArray array)
{
  jint size = (*env)->GetArrayLength(env, array);
  jbyte* elements = (*env)->GetByteArrayElements(env, array, NULL);
  printf("%.*s\n", size, elements);
  // Since we didn't modify the array there is no point in copying it back
  (*env)->ReleaseByteArrayElements(env, array, elements, JNI_ABORT);
}

void Java_Test_printFields(JNIEnv *env, jobject obj)
{
  jclass classTest;
  jclass objClass;
  jfieldID id;
  jboolean z;
  jint i;
  jlong l;
  jfloat f;
  jdouble d;
  jshort s;
  jbyte b;

  classTest = (*env)->FindClass(env, "Test");
  if (!classTest)
    printf("ERROR: Class Test not found!\n");

  if (!(*env)->IsInstanceOf(env, obj, classTest))
    printf("ERROR: IsInstanceOf\n");
  objClass = (*env)->GetObjectClass(env, obj);
  if (!(*env)->IsAssignableFrom(env, objClass, classTest))
    printf("ERROR: IsAssignableFrom\n");

  id = (*env)->GetFieldID(env, objClass, "z", "Z");
  z = (*env)->GetBooleanField(env, obj, id);
  Java_Test_println__Z(env, objClass, z);

  id = (*env)->GetFieldID(env, objClass, "i", "I");
  i = (*env)->GetIntField(env, obj, id);
  Java_Test_println__I(env, objClass, i);

  id = (*env)->GetFieldID(env, objClass, "l", "J");
  l = (*env)->GetLongField(env, obj, id);
  Java_Test_println__J(env, objClass, l);

  id = (*env)->GetFieldID(env, objClass, "f", "F");
  f = (*env)->GetFloatField(env, obj, id);
  Java_Test_println__F(env, objClass, f);

  id = (*env)->GetFieldID(env, objClass, "d", "D");
  d = (*env)->GetDoubleField(env, obj, id);
  Java_Test_println__D(env, objClass, d);

  id = (*env)->GetFieldID(env, objClass, "s", "S");
  s = (*env)->GetShortField(env, obj, id);
  Java_Test_println__I(env, objClass, s);

  id = (*env)->GetFieldID(env, objClass, "b", "B");
  b = (*env)->GetByteField(env, obj, id);
  Java_Test_println__I(env, objClass, b);
}
