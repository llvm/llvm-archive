#include <stdio.h>

#include "Test.h"

void Java_Test_print_1boolean_1ln(JNIEnv *env, jclass clazz, jboolean aBoolean)
{
  if (aBoolean)
    puts("true");
  else
    puts("false");
}

void Java_Test_print_1int_1ln(JNIEnv *env, jclass clazz, jint aInt)
{
  printf("%d\n", aInt);
}

void Java_Test_print_1long_1ln(JNIEnv *env, jclass clazz, jlong aLong)
{
  printf("%Ld\n", aLong);
}

void Java_Test_print_1float_1ln(JNIEnv *env, jclass clazz, jfloat aFloat)
{
  printf("%f\n", aFloat);
}

void Java_Test_print_1double_1ln(JNIEnv *env, jclass clazz, jdouble aDouble)
{
  printf("%f\n", aDouble);
}
