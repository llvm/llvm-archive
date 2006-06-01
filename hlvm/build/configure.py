from SCons.Environment import Environment as Environment
from SCons.Script.SConscript import SConsEnvironment as SConsEnvironment
from os.path import join as pjoin
from os.path import isdir as isdir
from os.path import isfile as isfile
from os.path import exists as exists
from os import environ as environ
from string import join as sjoin

def _failed(env):
  print "*** Configuration Check Failed. Required component missing"
  env.Exit(1)
  
def _getline(env,msg):
  response = raw_input(msg)
  if response == 'quit' or response == "exit":
    print "Configuration terminated by user"
    _failed(env)
  return response

def AskForDirs(context,pkgname,hdr,libs,progs=[]):
  hdrdir = _getline(context.env,
    'Enter directory containing %(name)s headers: ' % {'name':pkgname }
  )
  hdrpath = pjoin(hdrdir,hdr)
  if not isfile(hdrpath):
    print "Didn't find ",pkgname," headers in ",hdrpath,". Try again."
    return AskForDirs(context,pkgname,hdr,libs,progs)

  libdir = _getline(context.env,
    'Enter directory containing %(name)s libraries: ' % { 'name':pkgname }
  )
  for lib in libs:
    libpath = pjoin(libdir,context.env['LIBPREFIX'])
    libpath += lib
    libpath += context.env['LIBSUFFIX']
    if not isfile(libpath):
      print "Didn't find ",pkgname," libraries in ",libpath,". Try again."
      return AskForDirs(context,pkgname,hdr,libs,progs)

  progdir = None
  if len(progs) > 0:
    bindir = _getline(context.env,
      'Enter directory containing %(nm)s programs: ' % { 'nm':pkgname }
    )
  for prog in progs:
    binpath = pjoin(bindir,prog)
    if not isfile(binpath):
      print "Didn't find ",pkgname," programs in ",bindir,". Try again."
      return AskForDirs(context,pkgname,hdr,libs,progs)

  context.env[pkgname + '_lib'] = libdir
  context.env[pkgname + '_inc'] = hdrdir
  context.env[pkgname + '_bin'] = bindir
  context.env.AppendUnique(LIBPATH=[libdir],CPPPATH=[hdrdir],BINPATH=[bindir])

  return 1

def FindPackage(context,pkgname,hdr,libs,code='main(argc,argv);',paths=[],
                objs=[],hdrpfx='',progs=[]):
  msg = 'Checking for Package ' + pkgname + '...'
  context.Message(msg)
  lastLIBS = context.env['LIBS']
  lastLIBPATH = context.env['LIBPATH']
  lastCPPPATH = context.env['CPPPATH']
  lastBINPATH = context.env['BINPATH']
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
  # Check each path
  for p in paths:
    # Clear old settings from previous iterations
    libdir = None
    incdir = None
    bindir = None
    objects = []
    foundlib = 0
    # Check various library directory names
    for ldir in ['lib','libs','LIBS','Lib','Libs','Library','Libraries']:
      libdir = pjoin(p,ldir)
      # Skip this ldir if it doesn't exist
      if not isdir(libdir):
        continue

      # Check each required library to make sure its a file
      count = 0
      for alib in libs:
        library = pjoin(libdir,context.env['LIBPREFIX'])
        library += alib + context.env['LIBSUFFIX']
        if not isfile(library):
          break
        else:
          count += 1

      # if we didn't get them all then then try the next path
      if count != len(libs):
        continue

      # If we were given some object files to check
      if len(objs) > 0:
        count = 0
        # Check each of the required object files
        for o in objs:
          obj =  pjoin(libdir,context.env['OBJPREFIX'])
          obj += o + context.env['OBJSUFFIX']
          if not isfile(obj):
            break
          else:
            count += 1
            objects.append(obj)

        # If we didn't find them all, try the next library path
        if count != len(objs):
          continue

      # Otherwise we found the right library path
      foundlib = 1
      break
    
    if not foundlib:
      continue

    # At this point we have a valid libdir. Now check the various include 
    # diretory names
    count = 0
    for incdir in ['include', 'inc', 'incl']:
      hdrdir = pjoin(p,incdir)
      if not isdir(hdrdir):
        continue
      if hdrpfx != '':
        hdrdir = pjoin(hdrdir,hdrpfx)
        if not isdir(hdrdir):
          continue
      header = pjoin(hdrdir,hdr)
      if not isfile(header):
        continue
      else:
        count += 1
        break

    # Check that we found the header file
    if count != 1:
      continue

    # Check for programs
    count = 0
    for pr in progs:
      bindir = pjoin(p,'bin')
      if not exists(pjoin(bindir,pr)):
        break
      else:
        count += 1

    # If we didn't find all the programs, try the next path
    if count != len(progs):
      continue

    # We found everything we're looking for, now test it out
    context.env.AppendUnique(LIBPATH = [libdir])
    context.env.AppendUnique(CPPPATH = [hdrdir])
    context.env.AppendUnique(BINPATH = [bindir])
    context.env.AppendUnique(LINKFLAGS = objects)
    ret = context.TryRun(
      prog_template % {'include':hdr,'code':code},'.cpp'
    )

    # If the test failed, reset the variables and try next path
    if not ret:
      context.env.Replace(LIBS=lastLIBS, LIBPATH=lastLIBPATH, 
        CPPPATH=lastCPPPATH, BINPATH=lastBINPATH, LINKFLAGS=lastLINKFLAGS)
      continue

    # Otherwise, we've succeded, unset temporary environment settings and
    # set up the the packages environment variables.

    context.env.Replace(LIBS=lastLIBS, LINKFLAGS=lastLINKFLAGS)
    context.env[pkgname + '_bin'] = bindir
    context.env[pkgname + '_lib'] = libdir
    context.env[pkgname + '_inc'] = hdrdir
    context.Result('Found: (' + hdrdir + ',' + libdir + ')')
    return [libdir,hdrdir,bindir]

  # After processing all paths, we didn't find it
  context.Result( 'Not Found' )

  # So, lets try manually asking for it
  return AskForDirs(context,pkgname,hdr,libs,progs)

def FindLLVM(conf,env):
  code = 'new llvm::Module("Name");'
  conf.FindPackage('LLVM','llvm/Module.h',['LLVMCore','LLVMSystem'],
     code,[env['with_llvm'],'/proj/install/llvm'],[],'',
     ['llvm2cpp','llvm-as','llc'] )

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
    _failed(env)
  if not conf.CheckCXXHeader('cassert'):
    _failed(env)
  if not conf.CheckCXXHeader('ios'):
    _failed(env)
  if not conf.CheckCXXHeader('iostream'):
    _failed(env)
  if not conf.CheckCXXHeader('istream'):
    _failed(env)
  if not conf.CheckCXXHeader('map'):
    _failed(env)
  if not conf.CheckCXXHeader('memory'):
    _failed(env)
  if not conf.CheckCXXHeader('new'):
    _failed(env)
  if not conf.CheckCXXHeader('ostream'):
    _failed(env)
  if not conf.CheckCXXHeader('string'):
    _failed(env)
  if not conf.CheckCXXHeader('vector'):
    _failed(env)
  if not conf.CheckCXXHeader('llvm/ADT/StringExtras.h'):
    _failed(env)
  if not conf.CheckCXXHeader('llvm/System/Path.h'):
    _failed(env)
  if not conf.CheckCHeader(['apr-1/apr.h','apr-1/apr_pools.h']):
    _failed(env)
  if not conf.CheckCHeader(['apr-1/apr.h','apr-1/apr_uri.h']):
    _failed(env)
  if not conf.CheckCHeader('libxml/parser.h'):
    _failed(env)
  if not conf.CheckCHeader('libxml/relaxng.h'):
    _failed(env)
  if not conf.CheckCHeader('libxml/xmlwriter.h'):
    _failed(env)
  return 1

def CheckForPrograms(conf,env):
  if not conf.CheckProgram('gperf','with_gperf'):
    _failed(env)
  if not conf.CheckProgram('llc','with_llc'):
    _failed(env)
  if not conf.CheckProgram('llvm-dis','with_llvmdis',[env['LLVM_bin']]):
    _failed(env)
  if not conf.CheckProgram('llvm-as','with_llvmas',[env['LLVM_bin']]):
    _failed(env)
  if not conf.CheckProgram('llvm-gcc','with_llvmgcc',[env['LLVM_bin']]):
    _failed(env)
  if not conf.CheckProgram('llvm-g++','with_llvmgxx',[env['LLVM_bin']]):
    _failed(env)
  if not conf.CheckProgram('llvm2cpp','with_llvm2cpp',[env['LLVM_bin']]):
    _failed(env)
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
