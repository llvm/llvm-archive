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
