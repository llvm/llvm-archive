from SCons.Environment import Environment as Environment
from environment import ProvisionEnvironment as ProvisionEnvironment
from configure import ConfigureHLVM as ConfigureHLVM
from os import path as path
from string import join as sjoin
from string import replace as strrepl
import glob
def GetBuildEnvironment(targets,arguments):
  env = Environment();
  env['HLVM_Copyright'] = 'Copyright (c) 2006 Reid Spencer'
  env['HLVM_Maintainer'] = 'Reid Spencer <rspencer@reidspencer>'
  env['HLVM_Version'] = '0.1svn'
  env['HLVM_SO_CURRENT'] = '0'
  env['HLVM_SO_REVISION'] = '1'
  env['HLVM_SO_AGE'] = '0'
  env['HLVM_SO_VERSION'] = env['HLVM_SO_CURRENT']+':'+env['HLVM_SO_REVISION']
  env['HLVM_SO_VERSION'] += ':' + env['HLVM_SO_AGE']
  ProvisionEnvironment(env,targets,arguments)
  return ConfigureHLVM(env)

def GetAllCXXFiles(env):
  dir = env.Dir('.').abspath
  dir = strrepl(dir,path.join(env['BuildDir'],''),'',1)
  p1 = glob.glob(path.join(dir,'*.cpp'))
  p2 = glob.glob(path.join(dir,'*.cxx'))
  p3 = glob.glob(path.join(dir,'*.C'))
  return env.Flatten([p1,p2,p3])

def GetRNGQuoteSource(env):
  from scons import filterbuilders
  return filterbuilders.RNGQuoteSource(env)

def GetRNGTokenizer(env):
  from scons import filterbuilders
  return filterbuilders.RNGTokenizer(env)

def join(one,two):
  return path.join([one,two])
