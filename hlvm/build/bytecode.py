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
      includes + defines + " -c -x c++ " + src + " -o " + tgt )
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
  bc = env.Action(BytecodeAction,BytecodeMessage)
  arch = env.Action(BytecodeArchiveAction,BytecodeArchiveMessage)
  bc_bldr = env.Builder(action=bc,suffix='bc',src_suffix='cpp',single_source=1)
  arch_bldr = env.Builder(
    action=arch,suffix='bca',src_suffix='bc',src_builder=bc_bldr)
  env.Append(BUILDERS = {'Bytecode':bc_bldr, 'BytecodeArchive':arch_bldr})
  return 1
