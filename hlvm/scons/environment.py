# HLVM scons Software Construction Specification
# 

# Import stuff we need from SCons package
from os.path import join as pjoin
from SCons.Options import Options as Options
from SCons.Options import BoolOption as BoolOption
from SCons.Options import PathOption as PathOption
from SCons.Environment import Environment as Environment
from SCons.Script.SConscript import SConsEnvironment as SConsEnvironment

def ProvisionEnvironment(env,targets,arguments):
  env.EnsurePythonVersion(2,3)
  env.EnsureSConsVersion(0,96)
  env.SetOption('implicit_cache',1)
  env.TargetSignatures('build')
  VariantName=''
  opts = Options('custom.py',arguments)
  opts.AddOptions(
    BoolOption('assrt','Include assertions in the code',1),
    BoolOption('debug','Build with debug options turned on',1),
    BoolOption('inline','Cause inline code to be inline',0),
    BoolOption('optimize','Build object files with optimization',0),
    BoolOption('profile','Generate profiling aware code',0),
    BoolOption('small','Generate smaller code rather than faster',0),
    BoolOption('config','Generation the configuration data',0),
    PathOption('prefix','Specify where to install HLVM','/usr/local')
  )
  opts.Update(env)
  env['CCFLAGS']  = ' -pipe -Wall -Wcast-align -Wpointer-arith'
  env['CXXFLAGS'] = ' -pipe -Wall -Wcast-align -Wpointer-arith -Wno-deprecated'
  env['CXXFLAGS']+= ' -Wold-style-cast -Woverloaded-virtual -ffor-scope'
  env['CXXFLAGS']+= ' -fno-operator-names'
  env['CPPDEFINES'] = { '__STDC_LIMIT_MACROS':None }
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

  if env['assrt'] == 1:
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
  env.BuildDir(pjoin(BuildDir,'hlvm'),'hlvm',duplicate=0)
  env.BuildDir(pjoin(BuildDir,'tools'),'tools',duplicate=0)
  env.Prepend(CPPPATH=[pjoin('#',BuildDir)])
  env.Prepend(CPPPATH=['#'])
  env.SConsignFile(pjoin(BuildDir,'sconsign'))

  opts.Save('options.cache', env)
  env.Help(opts.GenerateHelpText(env,sort=cmp))
  return env;
