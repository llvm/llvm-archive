from SCons.Environment import Environment as Environment
from SCons.Script.SConscript import SConsEnvironment as SConsEnvironment
from os.path import join as pjoin
from os.path import isdir as isdir
from os.path import isfile as isfile
from os.path import exists as exists
from os import environ as environ

def CheckProgram(context,progname,varname,moredirs=[]):
  ret = 0
  context.Message("Checking for " + progname + "...")
  PATH = environ['PATH']
  dirs = PATH.split(':') + moredirs
  for dir in dirs:
    fname = pjoin(dir,progname)
    if exists(fname) and isfile(fname):
      context.env[varname] = fname
      ret = 1
      break;
  context.Result(ret)
  return ret

  
def AskForDirs(context,pkgname,hdr,lib):
  hdrdir = _getline(context.env,
    'Enter directory containing %(name)s headers: ' % {'name':pkgname }
  )
  hdrpath = pjoin(hdrdir,hdr)
  if isfile(hdrpath):
    libdir = _getline(context.env,
      'Enter directory containing %(name)s libraries: ' % { 'name':pkgname }
    )
    libpath = pjoin(libdir,context.env['LIBPREFIX'])
    libpath += lib
    libpath += context.env['LIBSUFFIX']
    if isfile(libpath):
      context.env[pkgname + '_lib'] = libdir
      context.env[pkgname + '_inc'] = hdrdir
      context.env.AppendUnique(LIBPATH=[libdir],CPPPATH=[hdrdir])
      return 1
    else:
      print "Didn't find ",pkgname," libraries in ",libpath,". Try again."
      return AskForDirs(context,pkgname,hdr,lib)
  else:
    print "Didn't find ",pkgname," headers in ",hdrpath,". Try again."
    return AskForDirs(context,pkgname,hdr,lib)

def FindPackage(context,pkgname,hdr,libs,code='main(argc,argv);',paths=[],
                objs=[], hdrpfx=''):
  msg = 'Checking for ' + pkgname + '...'
  context.Message(msg)
  lastLIBS = context.env['LIBS']
  lastLIBPATH = context.env['LIBPATH']
  lastCPPPATH= context.env['CPPPATH']
  lastLINKFLAGS = context.env['LINKFLAGS']
  prog_template = """
#include <%(include)s>
int main(int argc, char **argv) { 
  %(code)s
  return 0;
}
"""
  context.env.AppendUnique(LIBS = libs)
  paths += ['/proj','/proj/install','/opt/','/sw','/usr/local','/usr','/']
  for p in paths:
    for ldir in ['lib','bin','libexec','libs','LIBS']:
      libdir = pjoin(p,ldir)
      if not isdir(libdir):
        continue
      for alib in libs:
        library = pjoin(libdir,context.env['LIBPREFIX'])
        library += alib + context.env['LIBSUFFIX']
        if not exists(library):
          continue
        objects = " "
        count = 0
        for o in objs:
          obj =  pjoin(libdir,context.env['OBJPREFIX'])
          obj += o + context.env['OBJSUFFIX']
          if not exists(obj):
            continue;
          else:
            count += 1
            objects += obj + " "
        if count != len(objs):
          continue
        for incdir in ['include', 'inc', 'incl']:
          hdrdir = pjoin(p,incdir)
          if not isdir(hdrdir):
            continue
          if hdrpfx != '':
            hdrdir = pjoin(hdrdir,hdrpfx)
            if not exists(hdrdir):
              continue
          header = pjoin(hdrdir,hdr)
          if not exists(hdrdir):
            continue
          context.env.AppendUnique(LIBPATH = [libdir])
          context.env.AppendUnique(CPPPATH = [hdrdir])
          context.env.AppendUnique(LINKFLAGS = objects)
          ret = context.TryRun(
            prog_template % {'include':hdr,'code':code},'.cpp'
          )
          if not ret:
            context.env.Replace(LIBS=lastLIBS, LIBPATH=lastLIBPATH, 
              CPPPATH=lastCPPPATH, LINKFLAGS=lastLINKFLAGS)
            continue
          context.env[pkgname + '_lib'] = libdir
          context.env[pkgname + '_inc'] = hdrdir
          context.Result('Found: (' + hdrdir + ',' + libdir + ')')
          return [libdir,hdrdir]
  context.Result( 'Not Found' )
  return AskForDirs(context,pkgname,hdr,libs)

def _getline(env,msg):
  response = raw_input(msg)
  if response == 'quit' or response == "exit":
    print "Configuration terminated by user"
    env.Exit(1)
  return response

def FindLLVM(conf,env):
  code = 'llvm::Module* M = new llvm::Module("Name");'
  return conf.FindPackage('LLVM','llvm/Module.h',['LLVMSupport','LLVMSystem'],
      code,['/proj/install/llvm'],['LLVMCore','LLVMbzip2'])

def FindAPR(conf,env):
  code = 'apr_initialize();'
  return conf.FindPackage('APR',pjoin('apr-1','apr_general.h'),['apr-1'],code)

def FindAPRU(conf,env):
  code = 'apu_version_string();'
  return conf.FindPackage('APRU',pjoin('apr-1','apu_version.h'),['aprutil-1'],
      code)

def FindLibXML2(conf,env):
  code = 'xmlNewParserCtxt();'
  return conf.FindPackage('LIBXML2',pjoin('libxml','parser.h'),['xml2'],code,
    [],[],'libxml2')

def CheckStdCXXHeaders(conf,env):
  if not conf.CheckCXXHeader('vector'):
    env.Exit(1)
  if not conf.CheckCXXHeader('map'):
    env.Exit(1)
  if not conf.CheckCXXHeader('cassert'):
    env.Exit(1)
  if not conf.CheckCXXHeader('iostream'):
    env.Exit(1)
  return 1

def CheckForPrograms(conf,env):
  if not conf.CheckProgram('gperf','GPERF'):
    env.Exit(1)
  return 1

#dnl AC_PATH_PROG(path_EGREP, egrep, egrep)
#dnl AC_PATH_PROG(path_GPP, g++, g++)
#dnl AC_PATH_PROG(path_GPROF, gprof, gprof)
#dnl AC_PATH_PROG(path_PERL, perl, perl)
#dnl AC_PATH_PROG(path_PKGDATA, pkgdata, pkgdata)
#dnl AC_PATH_PROG(path_SORT, sort, sort)
#dnl AC_PATH_PROG(path_UNIQ, uniq, uniq)

def ConfigureHLVM(env):
  conf = env.Configure(custom_tests = { 
    'FindPackage':FindPackage, 'AskForDirs':AskForDirs,
    'CheckProgram':CheckProgram },
    conf_dir=pjoin(env['BuildDir'],'conftest'),
    log_file=pjoin(env['BuildDir'],'config.log')
  )
  env['LIBS'] = ""
  CheckForPrograms(conf,env)
  CheckStdCXXHeaders(conf,env)
  FindLibXML2(conf,env)
  FindAPR(conf,env)
  FindAPRU(conf,env)
  FindLLVM(conf,env)
  return conf.Finish()
