from SCons.Environment import Environment as Environment
from SCons.Defaults import Mkdir
from SCons.Defaults import Copy as Copy
import re,fileinput,os,glob
from string import join as sjoin
from os.path import join as pjoin
from os.path import exists

def getHeaders(env):
  context = pjoin(env['AbsSrcRoot'],'hlvm')
  result = []
  for d in glob.glob(pjoin(context,'*')):
    if os.path.isdir(d):
      for f in glob.glob(pjoin(context,d,'*.h')):
        if not os.path.isdir(f):
          result.append(f)
  return result

def DoxygenMessage(target,source,env):
  return "Creating API Documentation With Doxygen (be patient)"

def DoxygenAction(target,source,env):
  if env['with_doxygen'] == None:
    print "Documentation generation disabled because 'doxygen' was not found"
    return 0
  tgtdir = target[0].dir.path
  srcpath = source[0].path
  tgtpath = target[0].path
  docsdir = target[0].dir.path
  tarpath = pjoin(docsdir,'apis')

  env.Depends(tgtpath,srcpath)
  env.Depends(tgtpath,'doxygen.footer')
  env.Depends(tgtpath,'doxygen.header')
  env.Depends(tgtpath,'doxygen.intro')
  env.Depends(tgtpath,'doxygen.css')
  for f in getHeaders(env):
    env.Depends(tgtpath,f)
  if 0 == env.Execute(env['with_doxygen'] + ' ' + srcpath + ' >' + 
      pjoin(tgtdir,'doxygen.out')):
    return env.Execute(env['TAR'] + ' zcf ' + tgtpath + ' -C ' + tarpath + 
    ' html')
  return 0

def Doxygen(env):
  doxyAction = env.Action(DoxygenAction,DoxygenMessage)
  doxygenBuilder = env.Builder(action=doxyAction)
  env.Append(BUILDERS = {'Doxygen':doxygenBuilder} )
  return 1

def DoxygenInstallMessage(target,source,env):
  return "Installing API Documentation Into Subversion"

def DoxygenInstallAction(target,source,env):
  tarfile = target[0].path
  tgtdir  = target[0].dir.path
  srcpath = source[0].path
  env.Execute(Copy(tarfile,srcpath))
  env.Execute(env['TAR'] + ' zxf ' + tarfile + ' -C ' + tgtdir )
  return 0

def DoxygenInstall(env):
  doxyInstAction = env.Action(DoxygenInstallAction,DoxygenInstallMessage)
  doxyInstBuilder = env.Builder(action=doxyInstAction)
  env.Append(BUILDERS = {'DoxygenInstall':doxyInstBuilder} )
  return 1;

def XSLTMessage(target,source,env):
  return "Creating " + target[0].path + " via XSLT from " + source[0].path

def XSLTAction(target,source,env):
  env.Execute( env['with_xsltproc'] + ' ' + source[0].path + ' ' + 
    source[1].path + ' >' + target[0].path )

def XSLTproc(env):
  xsltAction = env.Action(XSLTAction,XSLTMessage)
  xsltBuilder = env.Builder(action=xsltAction)
  env.Append(BUILDERS = {'XSLTproc':xsltBuilder} )
