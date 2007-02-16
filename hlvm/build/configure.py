from SCons.Environment import Environment as Environment
from SCons.Script.SConscript import SConsEnvironment as SConsEnvironment
from os.path import join as pjoin
from os.path import isdir as isdir
from os.path import isfile as isfile
from os.path import exists as exists
from os import environ as environ
from string import join as sjoin

def AskForDir(env,pkg):
  response = raw_input('Enter directory containing %(pkg)s: ' % {'pkg':pkg })
  if response == 'quit' or response == "exit":
    print "Configuration terminated by user"
    env.Exit(1)
  if not isdir(response):
    print "'" + response + "' is not a directory. Try again."
    return AskForDir(env,pkg)
  return response

def FindPackage(ctxt,pkgname,varname,hdr,libs,code='main(argc,argv);',paths=[],
                objs=[],hdrpfx='',progs=[]):
  ctxt.Message('Checking for Package ' + pkgname + '...')
  ctxt.env[pkgname + '_bin'] = ''
  ctxt.env[pkgname + '_lib'] = ''
  ctxt.env[pkgname + '_inc'] = ''
  lastLIBS = ctxt.env['LIBS']
  lastLIBPATH = ctxt.env['LIBPATH']
  lastCPPPATH = ctxt.env['CPPPATH']
  lastBINPATH = ctxt.env['BINPATH']
  lastLINKFLAGS = ctxt.env['LINKFLAGS']
  prog_template = """
#include <%(include)s>
int main(int argc, char **argv) { 
  %(code)s
  return 0;
}
"""
  ctxt.env.AppendUnique(LIBS = libs)
  if len(ctxt.env['confpath']) > 0:
    paths = ctxt.env['confpath'].split(':') + paths
  if len(ctxt.env[varname]) > 0:
    paths = [ctxt.env[varname]] + paths
  paths += [ '/opt/local','/opt/','/sw/local','/sw','/usr/local','/usr']
  # Check each path
  for p in paths:
    # Clear old settings from previous iterations
    libdir = ''
    incdir = ''
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
        library = pjoin(libdir,ctxt.env['LIBPREFIX'])
        library += alib + ctxt.env['LIBSUFFIX']
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
          obj =  pjoin(libdir,ctxt.env['OBJPREFIX'])
          obj += o + ctxt.env['OBJSUFFIX']
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
    ctxt.env.AppendUnique(LIBPATH = [libdir])
    ctxt.env.AppendUnique(CPPPATH = [hdrdir])
    ctxt.env.AppendUnique(BINPATH = [bindir])
    ctxt.env.AppendUnique(LINKFLAGS = objects)
    ret = ctxt.TryRun(
      prog_template % {'include':hdr,'code':code},'.cpp'
    )

    # If the test failed, reset the variables and try next path
    if not ret:
      ctxt.env.Replace(LIBS=lastLIBS, LIBPATH=lastLIBPATH, 
        CPPPATH=lastCPPPATH, BINPATH=lastBINPATH, LINKFLAGS=lastLINKFLAGS)
      continue

    # Otherwise, we've succeded, unset temporary environment settings and
    # set up the the packages environment variables.

    ctxt.env.Replace(LIBS=lastLIBS, LINKFLAGS=lastLINKFLAGS)
    ctxt.env[varname] = p
    ctxt.env[pkgname + '_bin'] = bindir
    ctxt.env[pkgname + '_lib'] = libdir
    ctxt.env[pkgname + '_inc'] = hdrdir
    ctxt.Result('Found: (' + hdrdir + ',' + libdir + ')')
    return 1

  # After processing all paths, we didn't find it
  ctxt.Result( 'Not Found' )

  # We didn't find it. Lets ask for a directory name and call ourselves 
  # again using that directory name
  dir = AskForDir(ctxt.env,pkgname)
  return FindPackage(ctxt,pkgname,varname,hdr,libs,code,[dir],objs,hdrpfx,progs)

def FindLLVM(conf):
  code = 'new llvm::Module("Name");'
  conf.FindPackage('LLVM','with_llvm','llvm/Module.h',['LLVMCore','LLVMSystem'],
    code,[],[],'',['llvm2cpp','llvm-as','llc'] )

def FindAPR(conf):
  code = 'apr_initialize();'
  return conf.FindPackage('APR','with_apr',pjoin('apr-1','apr_general.h'),
    ['apr-1'],code,[])

def FindAPRU(conf):
  code = 'apu_version_string();'
  return conf.FindPackage('APRU','with_apru',pjoin('apr-1','apu_version.h'),
    ['aprutil-1'],code,[])

def FindLibXML2(conf):
  code = 'xmlNewParserCtxt();'
  return conf.FindPackage('LIBXML2','with_libxml2',pjoin('libxml','parser.h'),
    ['xml2'],code,[],[],'libxml2',['xmllint'])

def CheckProgram(ctxt,progname,varname,moredirs=[],critical=1):
  ctxt.Message("Checking for Program " + progname + "...")
  if ctxt.env[varname] is not None and exists(ctxt.env[varname]):
    ret = 1
  else:
    paths = sjoin(moredirs,':') + ':' + ctxt.env['ENV']['PATH'] 
    fname = ctxt.env.WhereIs(progname,paths)
    ret = fname != None
    if ret:
      ctxt.env[varname] = fname
    else:
      ctxt.env[varname] = None
  ctxt.Result(ret)
  if critical and not ret:
    print "Required Program '" + progname + "' is missing."
    ctxt.env.Exit(1)
  return ret

def CheckCXXHdr(conf,hdr,critical=1):
  ret = conf.CheckCXXHeader(hdr)
  if critical and not ret:
    print "Required C++ Header <" + hdr + "> is missing."
    conf.env.Exit(1)
  return ret

def CheckCHdr(conf,hdr,critical=1):
  ret = conf.CheckCHeader(hdr)
  if critical and not ret:
    print "Required C Header <" + hdr + "> is missing."
    conf.env.Exit(1)
  return ret
    

def CheckForHeaders(conf):
  CheckCXXHdr(conf,'algorithm')
  CheckCXXHdr(conf,'cassert')
  CheckCXXHdr(conf,'ios')
  CheckCXXHdr(conf,'iostream')
  CheckCXXHdr(conf,'istream')
  CheckCXXHdr(conf,'map')
  CheckCXXHdr(conf,'memory')
  CheckCXXHdr(conf,'new')
  CheckCXXHdr(conf,'ostream')
  CheckCXXHdr(conf,'string')
  CheckCXXHdr(conf,'vector')
  CheckCXXHdr(conf,'llvm/ADT/StringExtras.h')
  CheckCXXHdr(conf,'llvm/System/Path.h')
  CheckCHdr(conf,['apr-1/apr.h','apr-1/apr_pools.h'])
  CheckCHdr(conf,['apr-1/apr.h','apr-1/apr_uri.h'])
  CheckCHdr(conf,'libxml/parser.h')
  CheckCHdr(conf,'libxml/relaxng.h')
  CheckCHdr(conf,'libxml/xmlwriter.h')
  return 1

def CheckForPrograms(conf):
  conf.CheckProgram('g++','with_gxx')
  conf.CheckProgram('llc','with_llc',[conf.env['LLVM_bin']])
  conf.CheckProgram('llvm-dis','with_llvmdis',[conf.env['LLVM_bin']])
  conf.CheckProgram('llvm-as','with_llvmas',[conf.env['LLVM_bin']])
  conf.CheckProgram('llvm2cpp','with_llvm2cpp',[conf.env['LLVM_bin']])
  conf.CheckProgram('llvm-ar','with_llvmar',[conf.env['LLVM_bin']])
  conf.CheckProgram('llvm-ld','with_llvm_ld',[conf.env['LLVM_bin']])
  conf.CheckProgram('llvm-gcc','with_llvmgcc')
  conf.CheckProgram('llvm-g++','with_llvmgxx')
  conf.CheckProgram('gperf','with_gperf')
  conf.CheckProgram('pod2html','with_pod2html',[],0)
  conf.CheckProgram('pod2man','with_pod2man',[],0)
  conf.CheckProgram('xmllint','with_xmllint',['LIBXML2_bin'],0)
  if not conf.CheckProgram('runtest','with_runtest',[],0):
    print "*** TESTING DISABLED ***"
  if not conf.CheckProgram('doxygen','with_doxygen',[],0):
    print "*** DOXYGEN DISABLED ***"
  if not conf.CheckProgram('xsltproc','with_xsltproc',[],0):
    print "*** XSLTPROC DISABLED ***"
  return 1

def ConfigureHLVM(env):
  save_path = env['ENV']['PATH']
  conf = env.Configure(custom_tests = { 
    'FindPackage':FindPackage, 'CheckProgram':CheckProgram },
    conf_dir=pjoin(env['BuildDir'],'conftest'),
    log_file=pjoin(env['BuildDir'],'config.log'),
    config_h=pjoin(env['BuildDir'],'hlvm','Base','config.h')
  )
  rlist = []
  for p in env['confpath'].split(':'):
    if p != '' and exists(p) and exists(pjoin(p,'bin')):
      rlist = [p] + rlist
  for p in rlist:
    env.PrependENVPath('PATH', pjoin(p,'bin'))

  env['LIBS'] = ''

  FindLLVM(conf)
  FindAPR(conf)
  FindAPRU(conf)
  FindLibXML2(conf)
  CheckForHeaders(conf)
  CheckForPrograms(conf)
  conf.Finish()
  env['ENV']['PATH'] = save_path 
