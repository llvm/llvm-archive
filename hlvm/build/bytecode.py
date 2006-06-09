from SCons.Environment import Environment as Environment
import re,fileinput,os
from string import join as sjoin
from os.path import join as pjoin

def AsmFromBytecodeMessage(target,source,env):
  return "Generating Native Assembly From LLVM Bytecode" + source[0].path

def AsmFromBytecodeAction(target,source,env):
  theAction = env.Action(env['with_llc'] + ' -f -fast -o ' + target[0].path +
      ' ' + source[0].path)
  env.Depends(target,env['with_llc'])
  return env.Execute(theAction)

def BytecodeFromAsmMessage(target,source,env):
  return "Generating Bytecode From LLVM Assembly " + source[0].path

def BytecodeFromAsmAction(target,source,env):
  theAction = env.Action(env['with_llvmas'] + 
      ' -f -o ' + target[0].path + ' ' + source[0].path + ' ' + 
      env['LLVMASFLAGS'])
  env.Depends(target,env['with_llvmas'])
  return env.Execute(theAction);

def BytecodeFromCppMessage(target,source,env):
  return "Generating Bytecode From C++ Source " + source[0].path

def BytecodeFromCppAction(target,source,env):
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
    "PATH='" + env['LLVM_bin'] + "' " + env['with_llvmgxx'] + ' $CXXFLAGS ' + 
    includes + defines + " -c -emit-llvm -g -O3 -x c++ " + src + " -o " + tgt )
  env.Depends(target,env['with_llvmgxx'])
  return env.Execute(theAction);

def BytecodeArchiveMessage(target,source,env):
  return "Generating Bytecode Archive From Bytecode Modules"

def BytecodeArchiveAction(target,source,env):
  sources = ''
  for src in source:
    sources += ' ' + src.path
  theAction = env.Action(
    env['with_llvmar'] + ' cr ' + target[0].path + sources)
  env.Depends(target[0],env['with_llvmar'])
  return env.Execute(theAction);

def Bytecode(env):
  bc2s = env.Action(AsmFromBytecodeAction,AsmFromBytecodeMessage)
  ll2bc = env.Action(BytecodeFromAsmAction,BytecodeFromAsmMessage)
  cpp2bc = env.Action(BytecodeFromCppAction,BytecodeFromCppMessage)
  arch = env.Action(BytecodeArchiveAction,BytecodeArchiveMessage)
  bc2s_bldr = env.Builder(action=bc2s,suffix='s',src_suffix='bc',
              single_source=1)
  ll2bc_bldr  = env.Builder(action=ll2bc,suffix='bc',src_suffix='ll',
              single_source=1)
  cpp2bc_bldr = env.Builder(action=cpp2bc,suffix='bc',src_suffix='cpp',
              single_source=1)
  arch_bldr = env.Builder(action=arch,suffix='bca',src_suffix='bc',
      src_builder=[ cpp2bc_bldr,ll2bc_bldr])
  env.Append(BUILDERS = {
      'll2bc':ll2bc_bldr, 'cpp2bc':cpp2bc_bldr, 'bc2s':bc2s_bldr, 
      'BytecodeArchive':arch_bldr
  })
  return 1
