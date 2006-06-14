from SCons.Options import Options as Options
from SCons.Options import BoolOption as BoolOption
from SCons.Options import PathOption as PathOption
from SCons.Options import PackageOption as PackageOption
from SCons.Script.SConscript import SConsEnvironment as SConsEnvironment
from SCons.Script import COMMAND_LINE_TARGETS as COMMAND_LINE_TARGETS
from SCons.Environment import Environment as Environment
from SCons.Defaults import Mkdir as Mkdir
from configure import ConfigureHLVM as ConfigureHLVM
from os.path import join as pjoin
from os.path import exists as exists
from string import join as sjoin
from string import replace as strrepl
import glob
import datetime

def GetFiles(env,pat):
  prefix = env.Dir('.').abspath
  modprefix = strrepl(prefix,pjoin(env['BuildDir'],''),'',1)
  files = glob.glob(pjoin(modprefix,pat))
  result = []
  for f in files:
    result += [strrepl(f,modprefix,prefix,1)]
  return result

def GetAllCXXFiles(env):
  return GetFiles(env,'*.cpp')

def GetRNGQuoteSource(env):
  from build import filterbuilders
  return filterbuilders.RNGQuoteSource(env)

def GetCpp2LLVMCpp(env):
  from build import codegen
  return codegen.Cpp2LLVMCpp(env)

def GetRNGTokenizer(env):
  from build import filterbuilders
  return filterbuilders.RNGTokenizer(env)

def GetBytecode(env):
  from build import bytecode
  return bytecode.Bytecode(env)

def GetConfigFile(env):
  from build import filterbuilders
  return filterbuilders.ConfigFile(env)

def GetDoxygen(env):
  from build import documentation
  return documentation.Doxygen(env)

def GetDoxygenInstall(env):
  from build import documentation
  return documentation.DoxygenInstall(env)

def GetXSLTproc(env):
  from build import documentation
  return documentation.XSLTproc(env)

def GetPodGen(env):
  from build import documentation
  return documentation.PodGen(env)

def Dirs(env,dirlist=[]):
  dir = env.Dir('.').path
  if (dir == env.Dir('#').path):
    dir = '#' + env['BuildDir']
  else:
    dir = '#' + dir
  for d in dirlist:
    sconsfile = pjoin(dir,d,'SConscript')
    env.SConscript(sconsfile)

def InstallProgram(env,progs):
  if 'install' in COMMAND_LINE_TARGETS:
    dir = pjoin(env['prefix'],'bin')
    if not exists(dir):
      env.Execute(Mkdir(dir))
    env.Install(dir=env.Dir(dir),source=progs)
  if 'check' in COMMAND_LINE_TARGETS:
    env.Depends('check',progs)
  return 1

def InstallLibrary(env,libs):
  env.AppendUnique(LIBPATH=[env.Dir('.')])
  if 'install' in COMMAND_LINE_TARGETS:
    libdir = pjoin(env['prefix'],'lib')
    if not exists(libdir):
      env.Execute(Mkdir(libdir))
    env.Install(dir=env.Dir(libdir),source=libs)
  return 1

def InstallHeader(env,hdrs):
  if 'install' in COMMAND_LINE_TARGETS:
    moddir = strrepl(env.Dir('.').path,pjoin(env['BuildDir'],''),'',1)
    dir = pjoin(env['prefix'],'include',moddir)
    if not exists(dir):
      env.Execute(Mkdir(dir))
    env.Install(dir=env.Dir(dir),source=hdrs)
  return 1

def InstallDocs(env,docs):
  if 'install' in COMMAND_LINE_TARGETS:
    moddir = strrepl(env.Dir('.').path,pjoin(env['BuildDir'],''),'',1)
    dir = pjoin(env['prefix'],moddir)
    if not exists(dir):
      env.Execute(Mkdir(dir))
    env.Install(dir=dir,source=docs)

def InstallMan(env,mans):
  if 'install' in COMMAND_LINE_TARGETS:
    dir = pjoin(env['prefix'],'docs','man','man1')
    if not exists(dir):
      env.Execute(Mkdir(dir))
    env.Install(dir=dir,source=mans)


def GetBuildEnvironment(targets,arguments):
  env = Environment();
  env.EnsurePythonVersion(2,3)
  env.EnsureSConsVersion(0,96)
  env.SetOption('implicit_cache',1)
  env.TargetSignatures('build')
  if 'mode' in arguments:
    buildname = arguments['mode']
  else:
    buildname = 'default'
  options_file = '.' + buildname + '_options'
  if not exists(options_file):
    opts = Options('.options_cache',arguments)
  else:
    opts = Options(options_file,arguments)
  opts.AddOptions(
    BoolOption('assertions','Include assertions in the code',1),
    BoolOption('debug','Build with debug options turned on',1),
    BoolOption('inline','Cause inline code to be inline',0),
    BoolOption('optimize','Build object files with optimization',0),
    BoolOption('profile','Generate profiling aware code',0),
    BoolOption('small','Generate smaller code rather than faster',0),
    BoolOption('strip','Strip executables of their symbols',0),
  )
  opts.Add('prefix','Specify where to install HLVM','/usr/local')
  opts.Add('confpath','Specify additional configuration dirs to search','')
  opts.Add('with_llvm','Specify where LLVM is located','/usr/local')
  opts.Add('with_apr','Specify where apr is located','/usr/local/apr')
  opts.Add('with_apru','Specify where apr-utils is located','/usr/local/apr')
  opts.Add('with_libxml2','Specify where LibXml2 is located','/usr/local')
  opts.Add('with_gxx','Specify where the GCC C++ compiler is located',
           '/usr/local/bin/g++')
  opts.Add('with_llc','Specify where the LLVM compiler is located',
           '/usr/local/bin/llc')
  opts.Add('with_gccld','Specify where the LLVM GCC Linker is located',
           '/usr/local/bin/gccld')
  opts.Add('with_llvmdis','Specify where the LLVM disassembler is located',
           '/usr/local/bin/llvm-dis')
  opts.Add('with_llvmas','Specify where the LLVM assembler is located',
           '/usr/local/bin/llvm-as')
  opts.Add('with_llvmgcc','Specify where the LLVM C compiler is located',
           '/usr/local/bin/llvm-gcc')
  opts.Add('with_llvmgxx','Specify where the LLVM C++ compiler is located',
           '/usr/local/bin/llvm-g++')
  opts.Add('with_llvmar','Specify where the LLVM bytecode archiver is located',
           '/usr/local/bin/llvm-g++')
  opts.Add('with_llvm2cpp','Specify where the LLVM llvm2cpp program is located',
           '/usr/local/bin/llvm2cpp')
  opts.Add('with_gperf','Specify where the gperf program is located',
           '/usr/local/bin/gperf')
  opts.Add('with_runtest','Specify where DejaGnu runtest program is located',
           '/usr/local/bin/runtest')
  opts.Add('with_doxygen','Specify where the doxygen program is located',
           '/usr/local/bin/doxygen')
  opts.Add('with_xsltproc','Specify where the XSLT processor is located',
           '/usr/local/bin/xsltproc')
  opts.Add('with_pod2html','Specify where the POD to HTML generator is located',
           '/usr/local/bin/pod2html')
  opts.Add('with_pod2man','Specify where the POD to MAN generator is located',
           '/usr/local/bin/pod2man')
  opts.Add('with_xmllint','Specify where the xmllint program is located',
           '/usr/local/bin/xmllint')
  opts.Update(env)
  env['HLVM_Copyright'] = 'Copyright (c) 2006 Reid Spencer'
  env['HLVM_Maintainer'] = 'Reid Spencer <rspencer@reidspencer>'
  env['HLVM_Version'] = '0.1'
  env['HLVM_SO_CURRENT'] = '0'
  env['HLVM_SO_REVISION'] = '1'
  env['HLVM_SO_AGE'] = '0'
  env['HLVM_SO_VERSION'] = env['HLVM_SO_CURRENT']+':'+env['HLVM_SO_REVISION']
  env['HLVM_SO_VERSION'] += ':' + env['HLVM_SO_AGE']
  env['CCFLAGS']  = ' -pipe -Wall -Wcast-align -Wpointer-arith -Wno-long-long'
  env['CCFLAGS'] += ' -pedantic'
  env['CXXFLAGS'] = ' -pipe -Wall -Wcast-align -Wpointer-arith -Wno-deprecated'
  env['CXXFLAGS']+= ' -Wold-style-cast -Woverloaded-virtual -Wno-unused'
  env['CXXFLAGS']+= ' -Wno-long-long -pedantic -fno-operator-names -ffor-scope'
  env['CXXFLAGS']+= ' -fconst-strings'
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

  if env['strip']:
    VariantName +='T'
    env['LINKFLAGS'] += ' -s'
  else:
    VariantName += 't'

  BuildDir = buildname 
  env['Variant'] = VariantName
  env['BuildDir'] = BuildDir
  env['AbsObjRoot'] = env.Dir('#' + BuildDir).abspath
  env['AbsSrcRoot'] = env.Dir('#').abspath
  env.Prepend(CPPPATH=[pjoin('#',BuildDir)])
  env.Prepend(CPPPATH=['#'])
  env['LIBPATH'] = []
  env['BINPATH'] = []
  env['LLVMASFLAGS'] = ''
  env['LLVM2CPPFLAGS'] = ''
  env['LLVMGXXFLAGS'] = ''
  env['LLVMGCCFLAGS'] = ''
  env.BuildDir(BuildDir,'#',duplicate=0)
  env.SConsignFile(pjoin(BuildDir,'sconsign'))
  if 'install' in COMMAND_LINE_TARGETS:
    env.Alias('install',[
      env.Dir(pjoin(env['prefix'],'bin')),
      env.Dir(pjoin(env['prefix'],'lib')),
      env.Dir(pjoin(env['prefix'],'include')),
      env.Dir(pjoin(env['prefix'],'docs'))
    ])

  env.Help("""
HLVM Build Environment

Usage Examples:: 
  scons             - to do a normal build
  scons --clean     - to remove all derived (built) objects
  scons check       - to run the DejaGnu test suite
  scons install     - to install HLVM to a target directory
  scons docs        - to generate the doxygen documentation

Options:
""" + opts.GenerateHelpText(env,sort=cmp))
  print "HLVM BUILD MODE: " + VariantName + " (" + buildname + ")"
  ConfigureHLVM(env)
  now = datetime.datetime.utcnow();
  env['HLVM_ConfigTime'] = now.ctime();
  opts.Save(options_file,env)
  return env
