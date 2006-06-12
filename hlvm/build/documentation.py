from SCons.Environment import Environment as Environment
from SCons.Defaults import Mkdir
from SCons.Defaults import Copy as Copy
import re,fileinput,os,glob
from string import join as sjoin
from os.path import join as pjoin
from os.path import exists
import os.path as path

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
  if not env.Execute(Copy(tarfile,srcpath)):
    return env.Execute(env['TAR'] + ' zxf ' + tarfile + ' -C ' + tgtdir )
  return 1

def DoxygenInstall(env):
  doxyInstAction = env.Action(DoxygenInstallAction,DoxygenInstallMessage)
  doxyInstBuilder = env.Builder(action=doxyInstAction)
  env.Append(BUILDERS = {'DoxygenInstall':doxyInstBuilder} )
  return 1;

def XSLTMessage(target,source,env):
  return "Creating " + target[0].path + " via XSLT from " + source[0].path

def XSLTAction(target,source,env):
  return env.Execute( env['with_xsltproc'] + ' ' + source[0].path + ' ' + 
    source[1].path + ' >' + target[0].path )

def XSLTproc(env):
  xsltAction = env.Action(XSLTAction,XSLTMessage)
  xsltBuilder = env.Builder(action=xsltAction)
  env.Append(BUILDERS = {'XSLTproc':xsltBuilder} )

def Pod2HtmlMessage(target,source,env):
  return "Generating HTML From POD: " + source[0].path 
  
def Pod2HtmlAction(target,source,env):
  title = path.splitext(path.basename(source[0].path))[0]
  return env.Execute( env['with_pod2html'] + ' --css=man.css --htmlroot=.' +
      ' --podpath=. --noindex --infile=' + source[0].path + 
      ' --outfile=' + target[0].path + ' --title="' + title + ' command"')

def Pod2ManMessage(target,source,env):
  return "Generating MAN Page From POD: " + source[0].path 
  
def Pod2ManAction(target,source,env):
  title = path.splitext(path.basename(source[0].path))[0]
  return env.Execute( env['with_pod2man'] + ' --release=CVS' +
    ' --center="HLVM Tools Manual" ' + source[0].path + ' ' + target[0].path )

def PodGen(env):
  p2hAction = env.Action(Pod2HtmlAction,Pod2HtmlMessage)
  p2hBuildr = env.Builder(action=p2hAction,suffix='.html',src_suffix='.pod',
      single_source=1)
  p2mAction = env.Action(Pod2ManAction,Pod2ManMessage)
  p2mBuildr = env.Builder(action=p2mAction,suffix='.1',src_suffix='.pod',
      single_source=1)
  env.Append(BUILDERS = {'Pod2Html':p2hBuildr, 'Pod2Man':p2mBuildr} )
