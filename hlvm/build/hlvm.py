from SCons.Options import Options as Options
from SCons.Options import BoolOption as BoolOption
from SCons.Options import PathOption as PathOption
from SCons.Script.SConscript import SConsEnvironment as SConsEnvironment
from SCons.Environment import Environment as Environment
from configure import ConfigureHLVM as ConfigureHLVM
from os.path import join as pjoin
from string import join as sjoin
from string import replace as strrepl
import glob

def GetAllCXXFiles(env):
  dir = env.Dir('.').abspath
  dir = strrepl(dir,pjoin(env['BuildDir'],''),'',1)
  p1 = glob.glob(pjoin(dir,'*.cpp'))
  p2 = glob.glob(pjoin(dir,'*.cxx'))
  p3 = glob.glob(pjoin(dir,'*.C'))
  return env.Flatten([p1,p2,p3])

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
    print "sconsfile=",sconsfile
    env.SConscript(sconsfile)

def InstallProgram(env,prog):
  dir = pjoin(env['prefix'],'bin')
  env.Install(dir,prog)
  env.Alias('install',dir)
  return 1

def InstallLibrary(env,lib):
  dir = pjoin(env['prefix'],'lib')
  env.Install(dir,lib)
  env.Alias('install',dir)
  return 1

def InstallHeader(env,hdrname):
  dir = pjoin(env['prefix'],'include')
  env.Install(dir,hdrname)
  env.Alias('install',dir)
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
    PathOption('prefix','Specify where to install HLVM','/usr/local')
  )
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
  env['CXXFLAGS']+= ' -fno-operator-names'
  env['CPPDEFINES'] = { '__STDC_LIMIT_MACROS':None }
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
    env.Append(CCFLAGS=' -g')
    env.Append(CXXFLAGS=' -g')
    env.Append(CPPDEFINES={'HLVM_DEBUG':None})
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
  else :
    VariantName+='o'
    env.Append(CCFLAGS=' -O1')
    env.Append(CXXFLAGS=' -O1')

  BuildDir = 'build.' + VariantName
  env['Variant'] = VariantName
  env['BuildDir'] = BuildDir
  env['AbsObjRoot'] = env.Dir(BuildDir).abspath
  env['AbsSrcRoot'] = env.Dir('#').abspath
  env['LIBPATH'] = [
    pjoin('#',BuildDir,'hlvm/Base'),
    pjoin('#',BuildDir,'hlvm/AST'),
    pjoin('#',BuildDir,'hlvm/Reader/XML'),
    pjoin('#',BuildDir,'hlvm/Writer/XML')
  ];
  env.Prepend(CPPPATH=[pjoin('#',BuildDir)])
  env.Prepend(CPPPATH=['#'])
  env.BuildDir(pjoin(BuildDir,'hlvm'),'hlvm',duplicate=0)
  env.BuildDir(pjoin(BuildDir,'tools'),'tools',duplicate=0)
  env.BuildDir(pjoin(BuildDir,'test'),'test',duplicate=0)
  env.SConsignFile(pjoin(BuildDir,'sconsign'))
  env.Help("""
HLVM Build Environment

Usage Examples:: 
  scons             - to do a normal build
  scons --clean     - to remove all derived (built) objects
  scons check       - to run the DejaGnu test suite
  scons install     - to install HLVM to a target directory

Options:                                                     (Default)
  debug=on/off      - include debug symbols and code?        (on)
  assrt=on/off      - include code assertions?               (on)
  optimize=on/off   - optimize generated code?               (off)
  inline=on/off     - make inline calls inline?              (off)
  small=on/off      - make smaller rather than faster code?  (off)
  profile=on/off    - generate code for gprof profiling?     (off)
  prefix=<path>     - specify where HLVM should be installed (/usr/local)
""")
  print "HLVM BUILD MODE:", VariantName
  ConfigureHLVM(env)
  if 'check' in targets:
    from build import check
    check.Check(env)
  return env

