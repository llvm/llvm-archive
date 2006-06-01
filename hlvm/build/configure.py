from SCons.Environment import Environment as Environment
from SCons.Script.SConscript import SConsEnvironment as SConsEnvironment
from os.path import join as pjoin
from os.path import isdir as isdir
from os.path import isfile as isfile
from os.path import exists as exists
from os import environ as environ
from string import join as sjoin

  
def _getline(env,msg):
  response = raw_input(msg)
  if response == 'quit' or response == "exit":
    print "Configuration terminated by user"
    env.Exit(1)
  return response

def AskForDirs(context,pkgname,hdr,libs):
  hdrdir = _getline(context.env,
    'Enter directory containing %(name)s headers: ' % {'name':pkgname }
  )
  hdrpath = pjoin(hdrdir,hdr)
  if isfile(hdrpath):
    libdir = _getline(context.env,
      'Enter directory containing %(name)s libraries: ' % { 'name':pkgname }
    )
    for lib in libs:
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
        return AskForDirs(context,pkgname,hdr,libs)
  else:
    print "Didn't find ",pkgname," headers in ",hdrpath,". Try again."
    return AskForDirs(context,pkgname,hdr,libs)

def FindPackage(context,pkgname,hdr,libs,code='main(argc,argv);',paths=[],
                objs=[],hdrpfx='',progs=[]):
  msg = 'Checking for Package ' + pkgname + '...'
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
  paths += [
    '/proj','/proj/install','/opt/local','/opt/','/sw','/usr/local','/usr','/'
  ]
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
        objects = []
        count = 0
        for o in objs:
          obj =  pjoin(libdir,context.env['OBJPREFIX'])
          obj += o + context.env['OBJSUFFIX']
          if not exists(obj):
            continue;
          else:
            count += 1
            objects.append(obj)
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
          else:
            context.env.Replace(LIBS=lastLIBS, LINKFLAGS=lastLINKFLAGS)
          ret = 1
          bindir = pjoin(p,'bin')
          for pr in progs:
            if not exists(pjoin(bindir,pr)):
              ret = 0
              break;
          if not ret:
            continue
          if len(progs) > 0:
            context.env[pkgname + '_bin'] = bindir
          context.env[pkgname + '_lib'] = libdir
          context.env[pkgname + '_inc'] = hdrdir
          context.Result('Found: (' + hdrdir + ',' + libdir + ')')
          return [libdir,hdrdir]
  context.Result( 'Not Found' )
  return AskForDirs(context,pkgname,hdr,libs)

def FindLLVM(conf,env):
  code = 'new llvm::Module("Name");'
  conf.FindPackage('LLVM','llvm/Module.h',['LLVMSupport','LLVMSystem'],
     code,[env['with_llvm'],'/proj/install/llvm'],['LLVMCore','LLVMbzip2'],
     '', ['llvm-as','llc'] )


def FindAPR(conf,env):
  code = 'apr_initialize();'
  return conf.FindPackage('APR',pjoin('apr-1','apr_general.h'),['apr-1'],code,
      [env['with_apr']])

def FindAPRU(conf,env):
  code = 'apu_version_string();'
  return conf.FindPackage('APRU',pjoin('apr-1','apu_version.h'),['aprutil-1'],
      code,[env['with_apru']])

def FindLibXML2(conf,env):
  code = 'xmlNewParserCtxt();'
  return conf.FindPackage('LIBXML2',pjoin('libxml','parser.h'),['xml2'],code,
    [env['with_xml2']],[],'libxml2')

def CheckProgram(context,progname,varname,moredirs=[]):
  context.Message("Checking for Program " + progname + "...")
  if exists(context.env[varname]):
    ret = 1
  else:
    paths = sjoin(moredirs,':') + environ['PATH'] 
    fname = context.env.WhereIs(progname,paths)
    ret = fname != None
    if ret:
      context.env[varname] = fname
  context.Result(ret)
  return ret

def CheckForHeaders(conf,env):
  if not conf.CheckCXXHeader('algorithm'):
    env.Exit(1)
  if not conf.CheckCXXHeader('cassert'):
    env.Exit(1)
  if not conf.CheckCXXHeader('ios'):
    env.Exit(1)
  if not conf.CheckCXXHeader('iostream'):
    env.Exit(1)
  if not conf.CheckCXXHeader('istream'):
    env.Exit(1)
  if not conf.CheckCXXHeader('map'):
    env.Exit(1)
  if not conf.CheckCXXHeader('memory'):
    env.Exit(1)
  if not conf.CheckCXXHeader('new'):
    env.Exit(1)
  if not conf.CheckCXXHeader('ostream'):
    env.Exit(1)
  if not conf.CheckCXXHeader('string'):
    env.Exit(1)
  if not conf.CheckCXXHeader('vector'):
    env.Exit(1)
  if not conf.CheckCXXHeader('llvm/ADT/StringExtras.h'):
    env.Exit(1)
  if not conf.CheckCXXHeader('llvm/System/Path.h'):
    env.Exit(1)
  if not conf.CheckCHeader(['apr-1/apr.h','apr-1/apr_pools.h']):
    env.Exit(1)
  if not conf.CheckCHeader(['apr-1/apr.h','apr-1/apr_uri.h']):
    env.Exit(1)
  if not conf.CheckCHeader('libxml/parser.h'):
    env.Exit(1)
  if not conf.CheckCHeader('libxml/relaxng.h'):
    env.Exit(1)
  if not conf.CheckCHeader('libxml/xmlwriter.h'):
    env.Exit(1)
  return 1

def CheckForPrograms(conf,env):
  if not conf.CheckProgram('gperf','with_gperf'):
    env.Exit(1)
  if not conf.CheckProgram('llc','with_llc'):
    env.Exit(1)
  if not conf.CheckProgram('llvm-dis','with_llvmdis',[env['LLVM_bin']]):
    env.Exit(1)
  if not conf.CheckProgram('llvm-as','with_llvmas',[env['LLVM_bin']]):
    env.Exit(1)
  if not conf.CheckProgram('llvm-gcc','with_llvmgcc',[env['LLVM_bin']]):
    env.Exit(1)
  if not conf.CheckProgram('llvm-g++','with_llvmgxx',[env['LLVM_bin']]):
    env.Exit(1)
  if not conf.CheckProgram('llvm2cpp','with_llvm2cpp',[env['LLVM_bin']]):
    env.Exit(1)
  if not conf.CheckProgram('runtest','with_runtest'):
    env['with_runtest'] = None
    print "*** TESTING DISABLED ***"
  if not conf.CheckProgram('doxygen','with_doxygen'):
    env['with_runtest'] = None
    print "*** DOXYGEN DISABLED ***"
  if not conf.CheckProgram('xsltproc','with_xsltproc'):
    env['with_runtest'] = None
    print "*** XSLTPROC DISABLED ***"
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
  FindLibXML2(conf,env)
  FindAPR(conf,env)
  FindAPRU(conf,env)
  FindLLVM(conf,env)
  CheckForPrograms(conf,env)
  CheckForHeaders(conf,env)
  conf.Finish()
