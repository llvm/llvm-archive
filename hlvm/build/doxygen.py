from SCons.Environment import Environment as Environment
from SCons.Defaults import Mkdir
import re,fileinput,os,glob
from string import join as sjoin
from os.path import join as pjoin
from os.path import exists

def getHeaders(env):
  context = pjoin(env['AbsSrcRoot'],'hlvm')
  result = []
  for d in glob.glob(pjoin(context,'*.h')):
    if not os.path.isdir(d):
      result.append(d)
  return result

def DoxygenAction(target,source,env):
  if env['with_doxygen'] == None:
    print "Documentation generation disabled because 'doxygen' was not found"
    return 0
  tgtdir = target[0].dir.path
  srcpath = source[0].path
  tgtpath = target[0].path
  env.Depends(tgtpath,srcpath)
  env.Depends(tgtpath,'doxygen.footer')
  env.Depends(tgtpath,'doxygen.header')
  env.Depends(tgtpath,'doxygen.intro')
  env.Depends(tgtpath,'doxygen.css')
  env.Depends(tgtpath,getHeaders(env))
  if env.Execute(env['with_doxygen'] + ' ' + srcpath + ' >' + 
      pjoin(tgtdir,'doxygen.out')):
    return env.Execute(env['TAR'] + ' zcf ' + tgtpath + ' ' + 
      pjoin(tgtdir,'apis'))
  return 0

def DoxygenMessage(target,source,env):
  return "Creating API Documentation With Doxygen (be patient)"

def Doxygen(env):
  doxyAction = env.Action(DoxygenAction,DoxygenMessage)
  doxygenBuilder = env.Builder(action=doxyAction)
  env.Append(BUILDERS = {'Doxygen':doxygenBuilder} )
  env.Alias('doxygen','doxygen.tar.gz')
  return 1
