from SCons.Options import Options as Options
from SCons.Options import BoolOption as BoolOption
from SCons.Options import PathOption as PathOption
from SCons.Options import PackageOption as PackageOption
from SCons.Script.SConscript import SConsEnvironment as SConsEnvironment
from SCons.Script import COMMAND_LINE_TARGETS as COMMAND_LINE_TARGETS
from SCons.Environment import Environment as Environment
from configure import ConfigureHLVM as ConfigureHLVM
from os.path import join as pjoin
from string import join as sjoin
from string import replace as strrepl
import glob

def GetFiles(env,pat,dir=''):
  spec = pjoin(env.Dir('.').abspath,dir,pat)
  spec = strrepl(spec,pjoin(env['BuildDir'],''),'',1)
  return glob.glob(spec)

def GetAllCXXFiles(env):
  return env.Flatten([GetFiles(env,'*.cpp'),GetFiles(env,'*.cxx')])

def GetRNGQuoteSource(env):
  from build import filterbuilders
  return filterbuilders.RNGQuoteSource(env)

def GetRNGTokenizer(env):
  from build import filterbuilders
  return filterbuilders.RNGTokenizer(env)

def Dirs(env,dirlist=[]):
  dir = env.Dir('.').path
  if (dir == env.Dir('#').path):
    dir = '#' + env['BuildDir']
  else:
    dir = '#' + dir
  for d in dirlist:
    sconsfile = pjoin(dir,d,'SConscript')
    env.SConscript(sconsfile)

def InstallProgram(env,prog):
  if 'install' in COMMAND_LINE_TARGETS:
    dir = pjoin(env['prefix'],'bin')
    env.Install(dir,prog)
  return 1

def InstallLibrary(env,lib):
  env.AppendUnique(LIBPATH=[env.Dir('.')])
  if 'install' in COMMAND_LINE_TARGETS:
    libdir = pjoin(env['prefix'],'lib')
    env.Install(dir,lib)
  return 1

def InstallHeader(env,hdrs):
  if 'install' in COMMAND_LINE_TARGETS:
    moddir = strrepl(env.Dir('.').path,pjoin(env['BuildDir'],''),'',1)
    dir = pjoin(env['prefix'],'include',moddir)
    env.Install(dir,hdrs)
  return 1

def GetBuildEnvironment(targets,arguments):
  env = Environment();
  env.EnsurePythonVersion(2,3)
  env.EnsureSConsVersion(0,96)
  env.SetOption('implicit_cache',1)
  env.TargetSignatures('build')
  opts = Options('.options_cache',arguments)
  opts.AddOptions(
    BoolOption('assertions','Include assertions in the code',1),
    BoolOption('debug','Build with debug options turned on',1),
    BoolOption('inline','Cause inline code to be inline',0),
    BoolOption('optimize','Build object files with optimization',0),
    BoolOption('profile','Generate profiling aware code',0),
    BoolOption('small','Generate smaller code rather than faster',0),
  )
  opts.Add('prefix','Specify where to install HLVM','/usr/local')
  opts.Add('with_llvm','Specify where LLVM is located','/usr/local'),
  opts.Add('with_apr','Specify where apr is located','/usr/local/apr'),
  opts.Add('with_apru','Specify where apr-utils is located','/usr/local/apr'),
  opts.Add('with_xml2','Specify where LibXml2 is located','/usr/local'),
  opts.Update(env)
  opts.Save('.options_cache',env)
  env['HLVM_Copyright'] = 'Copyright (c) 2006 Reid Spencer'
  env['HLVM_Maintainer'] = 'Reid Spencer <rspencer@reidspencer>'
  env['HLVM_Version'] = '0.1svn'
  env['HLVM_SO_CURRENT'] = '0'
  env['HLVM_SO_REVISION'] = '1'
  env['HLVM_SO_AGE'] = '0'
  env['HLVM_SO_VERSION'] = env['HLVM_SO_CURRENT']+':'+env['HLVM_SO_REVISION']
  env['HLVM_SO_VERSION'] += ':' + env['HLVM_SO_AGE']
  env['CCFLAGS']  = ' -pipe -Wall -Wcast-align -Wpointer-arith'
  env['CXXFLAGS'] = ' -pipe -Wall -Wcast-align -Wpointer-arith -Wno-deprecated'
  env['CXXFLAGS']+= ' -Wold-style-cast -Woverloaded-virtual -ffor-scope'
  env['CXXFLAGS']+= ' -fno-operator-names -Wno-unused'
  env['CPPDEFINES'] = { '__STDC_LIMIT_MACROS':None, '_GNU_SOURCE':None }
  VariantName=''
  if env['small'] == 1:
    VariantName='S'
    env.Append(CCFLAGS=' -Os')
    env.Append(CXXFLAGS=' -Os')
  else :
    VariantName='s'

  if env['profile'] == 1:
    VariantName+='P'
    env.Append(CCFLAGS=' -pg')
    env.Append(CXXFLAGS=' -pg')
  else :
    VariantName+='p'

  if env['assertions'] == 1:
    VariantName+='A'
    env.Append(CPPDEFINES={'HLVM_ASSERT':None})
  else :
    VariantName+='a'

  if env['debug'] == 1 :
    VariantName += 'D'
    env.Append(CCFLAGS=' -ggdb')
    env.Append(CXXFLAGS=' -ggdb')
    env.Append(CPPDEFINES={'HLVM_DEBUG':None})
    env.Append(LINKFLAGS='-ggdb')
  else :
    VariantName+='d'

  if env['inline'] == 1:
    VariantName+='I'
  else :
    VariantName+='i'
    env.Append(CXXFLAGS=' -fno-inline')

  if env['optimize'] == 1 :
    VariantName+='O'
    env.Append(CCFLAGS=' -O3')
    env.Append(CXXFLAGS=' -O3')
    env.Append(LINKFLAGS='-O3')
  else :
    VariantName+='o'

  BuildDir = 'build.' + VariantName
  env['Variant'] = VariantName
  env['BuildDir'] = BuildDir
  env['AbsObjRoot'] = env.Dir(BuildDir).abspath
  env['AbsSrcRoot'] = env.Dir('#').abspath
  env.Prepend(CPPPATH=[pjoin('#',BuildDir)])
  env.Prepend(CPPPATH=['#'])
  env['LIBPATH'] = []
  env.BuildDir(pjoin(BuildDir,'hlvm'),'hlvm',duplicate=0)
  env.BuildDir(pjoin(BuildDir,'tools'),'tools',duplicate=0)
  env.BuildDir(pjoin(BuildDir,'test'),'test',duplicate=0)
  env.BuildDir(pjoin(BuildDir,'docs'),'docs',duplicate=0)
  env.SConsignFile(pjoin(BuildDir,'sconsign'))
  if 'install' in COMMAND_LINE_TARGETS:
    env.Alias('install',pjoin(env['prefix'],'bin'))
    env.Alias('install',pjoin(env['prefix'],'lib'))
    env.Alias('install',pjoin(env['prefix'],'include'))
  env.Help("""
HLVM Build Environment

Usage Examples:: 
  scons             - to do a normal build
  scons --clean     - to remove all derived (built) objects
  scons check       - to run the DejaGnu test suite
  scons install     - to install HLVM to a target directory
  scons doxygen     - to generate the doxygen documentation

Options:
""" + opts.GenerateHelpText(env,sort=cmp))
  print "HLVM BUILD MODE:", VariantName
  ConfigureHLVM(env)
  return env

