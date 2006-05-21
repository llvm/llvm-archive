from SCons.Environment import Environment as Environment
from SCons.Script.SConscript import SConsEnvironment as SConsEnvironment

def ConfigureHLVM(env):
  conf = env.Configure()
  CheckStdCXXHeaders(conf,env)
  CheckForLibXML2(conf,env)
  CheckForAPR(conf,env)
  CheckForAPRU(conf,env)
  CheckForLLVM(conf,env)
  env = conf.Finish()

def CheckStdCXXHeaders(env):
  if not conf.CheckCXXHeader('vector'):
    env.Exit(1)
  if not conf.CheckCXXHeader('map'):
    env.Exit(1)
  if not conf.CheckCXXHeader('cassert'):
    env.Exit(1)
  if not conf.CheckCXXHeader('iostream'):
    env.Exit(1)

def CheckForLibXML2(conf,env):

def CheckForAPR(conf,env):

def CheckForAPRU(conf,env):

def CheckForLLVM(conf,env):
  if not conf.CheckLibWithHeader(
      libs='LLVMCore',
      header='llvm/Module.h',
      language='c++',
      call='llvm::Module* m = new llvm::Module("name")',
      autoadd=1):
    env.Exit(1)
  return env
