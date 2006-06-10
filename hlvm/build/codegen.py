from SCons.Environment import Environment as Environment
import re,fileinput,os
from string import join as sjoin
from os.path import join as pjoin

def CPP2LLVMCPPAction(target,source,env):
  funcName = os.path.splitext(os.path.basename(source[0].path))[0]
  src = source[0].path
  tgt = target[0].path
  theAction = env.Action(
    "PATH='" + env['LLVM_bin'] + "' " + env['with_llvmgxx'] + env['CXXFLAGS'] +
      " -c --emit-llvm -x c++ " + src + " -o - | " + 
    env['with_llvmdis'] + " -o - | " + 
    env['with_llvm2cpp'] + " " + env['LLVM2CPPFLAGS'] + " -o " + tgt
  )
  env.Depends(tgt,env['with_llvm2cpp'])
  env.Depends(tgt,env['with_llvmdis'])
  env.Execute(theAction);
  return 0

def CPP2LLVMCPPMessage(target,source,env):
  return "Generating LLVM IR C++ From C++ Input: " + source[0].path

def Cpp2LLVMCpp(env):
  a = env.Action(CPP2LLVMCPPAction,CPP2LLVMCPPMessage)
  b = env.Builder(action=a,suffix='inc',src_suffix='h',single_source=1)
  env.Append(BUILDERS = {'Cpp2LLVMCpp':b})
  env['LLVM2CPPFLAGS'] = "";
  return 1
