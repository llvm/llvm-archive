from SCons.Environment import Environment as Environment
import re,fileinput,os
from string import join as sjoin
from os.path import join as pjoin

def BytecodeAction(target,source,env):
  funcName = os.path.splitext(os.path.basename(source[0].path))[0]
  sources = ""
  for src in source:
    sources += " " + src.path
  tgt = target[0].path
  theAction = env.Action(
    "PATH='" + env['LLVM_bin'] + "' " + env['with_llvmgxx'] + 
      " -c -x c++ " + sources + " -o " + tgt)
  env.Depends(target,env['with_llvmgxx'])
  env.Execute(theAction);
  return 0

def BytecodeMessage(target,source,env):
  return "Generating Bytecode From C++ Source"

def Bytecode(env):
  a = env.Action(BytecodeAction,BytecodeMessage)
  b = env.Builder(action=a,suffix='bc',src_suffix='cpp')
  env.Append(BUILDERS = {'Bytecode':b})
  return 1
