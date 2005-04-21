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

void Java_Test_printStaticFields(JNIEnv *env, jclass clazz)
{
  jclass classTest;
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

  if (!(*env)->IsAssignableFrom(env, clazz, classTest))
    printf("ERROR: IsAssignableFrom\n");

  id = (*env)->GetStaticFieldID(env, clazz, "Z", "Z");
  z = (*env)->GetStaticBooleanField(env, clazz, id);
  Java_Test_println__Z(env, clazz, z);

  id = (*env)->GetStaticFieldID(env, clazz, "I", "I");
  i = (*env)->GetStaticIntField(env, clazz, id);
  Java_Test_println__I(env, clazz, i);

  id = (*env)->GetStaticFieldID(env, clazz, "L", "J");
  l = (*env)->GetStaticLongField(env, clazz, id);
  Java_Test_println__J(env, clazz, l);

  id = (*env)->GetStaticFieldID(env, clazz, "F", "F");
  f = (*env)->GetStaticFloatField(env, clazz, id);
  Java_Test_println__F(env, clazz, f);

  id = (*env)->GetStaticFieldID(env, clazz, "D", "D");
  d = (*env)->GetStaticDoubleField(env, clazz, id);
  Java_Test_println__D(env, clazz, d);

  id = (*env)->GetStaticFieldID(env, clazz, "S", "S");
  s = (*env)->GetStaticShortField(env, clazz, id);
  Java_Test_println__I(env, clazz, s);

  id = (*env)->GetStaticFieldID(env, clazz, "B", "B");
  b = (*env)->GetStaticByteField(env, clazz, id);
  Java_Test_println__I(env, clazz, b);
}

void Java_Test_printMethods(JNIEnv *env, jobject obj)
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

  id = (*env)->GetMethodID(env, objClass, "z", "()Z");
  z = (*env)->CallBooleanMethod(env, obj, id);
  Java_Test_println__Z(env, objClass, z);

  id = (*env)->GetMethodID(env, objClass, "i", "(I)I");
  i = (*env)->CallIntMethod(env, obj, id, 2);
  Java_Test_println__I(env, objClass, i);

  id = (*env)->GetMethodID(env, objClass, "j", "(BS)J");
  l = (*env)->CallLongMethod(env, obj, id, 23, 45);
  Java_Test_println__J(env, objClass, l);

  id = (*env)->GetMethodID(env, objClass, "f", "(B)F");
  f = (*env)->CallFloatMethod(env, obj, id, 123);
  Java_Test_println__F(env, objClass, f);

  id = (*env)->GetMethodID(env, objClass, "d", "(IJ)D");
  d = (*env)->CallDoubleMethod(env, obj, id, 654, 123ll);
  Java_Test_println__D(env, objClass, d);

  id = (*env)->GetMethodID(env, objClass, "s", "(DB)S");
  s = (*env)->CallShortMethod(env, obj, id, 2.0, 123);
  Java_Test_println__I(env, objClass, s);

  id = (*env)->GetMethodID(env, objClass, "b", "(SF)B");
  b = (*env)->CallByteMethod(env, obj, id, 23, -2.0);
  Java_Test_println__I(env, objClass, b);
}

void Java_Test_printStaticMethods(JNIEnv *env, jobject clazz)
{
  jclass classTest;
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

  if (!(*env)->IsAssignableFrom(env, clazz, classTest))
    printf("ERROR: IsAssignableFrom\n");

  id = (*env)->GetStaticMethodID(env, clazz, "Z", "()Z");
  z = (*env)->CallStaticBooleanMethod(env, clazz, id);
  Java_Test_println__Z(env, clazz, z);

  id = (*env)->GetStaticMethodID(env, clazz, "I", "(I)I");
  i = (*env)->CallStaticIntMethod(env, clazz, id, 2);
  Java_Test_println__I(env, clazz, i);

  id = (*env)->GetStaticMethodID(env, clazz, "J", "(BS)J");
  l = (*env)->CallStaticLongMethod(env, clazz, id, 23, 45);
  Java_Test_println__J(env, clazz, l);

  id = (*env)->GetStaticMethodID(env, clazz, "F", "(B)F");
  f = (*env)->CallStaticFloatMethod(env, clazz, id, 123);
  Java_Test_println__F(env, clazz, f);

  id = (*env)->GetStaticMethodID(env, clazz, "D", "(IJ)D");
  d = (*env)->CallStaticDoubleMethod(env, clazz, id, 654, 12354123ll);
  Java_Test_println__D(env, clazz, d);

  id = (*env)->GetStaticMethodID(env, clazz, "S", "(DB)S");
  s = (*env)->CallStaticShortMethod(env, clazz, id, 2.0, 123);
  Java_Test_println__I(env, clazz, s);

  id = (*env)->GetStaticMethodID(env, clazz, "B", "(SF)B");
  b = (*env)->CallStaticByteMethod(env, clazz, id, 21, -58.0);
  Java_Test_println__I(env, clazz, b);
}
