from SCons.Environment import Environment as Environment
import re,fileinput,os
from string import join as sjoin
from os.path import join as pjoin

def BytecodeMessage(target,source,env):
  return "Generating Bytecode From C++ Source"

def BytecodeAction(target,source,env):
  includes = ""
  for inc in env['CPPPATH']:
    if inc[0] == '#':
      inc = env['AbsSrcRoot'] + inc[1:]
    includes += " -I" + inc
  defines = ""
  for d in env['CPPDEFINES'].keys():
    if env['CPPDEFINES'][d] == None:
      defines += " -D" + d
    else:
      defines += " -D" + d + "=" + env['CPPDEFINES'][d]
  src = source[0].path
  tgt = target[0].path
  theAction = env.Action(
    "PATH='" + env['LLVM_bin'] + "' " + env['with_llvmgxx'] + ' -Wall' +
      includes + defines + " -g -c -x c++ " + src + " -o " + tgt )
  env.Depends(target,env['with_llvmgxx'])
  return env.Execute(theAction);

