# HLVM scons Software Construction Specification
# 

# Import stuff we need from SCons package
from string import join
from SCons.Options import Options as Options
from SCons.Options import BoolOption as BoolOption
from SCons.Environment import Environment as Environment
from SCons.Script.SConscript import SConsEnvironment as SConsEnvironment

def ProvisionEnvironment(env):
  env.EnsurePythonVersion(2,3)
  env.EnsureSConsVersion(0,96)
  env.SetOption('implicit_cache',1)
  env.TargetSignatures('build')
  opts = Options('custom.py')
  opts.AddOptions(
    BoolOption('assrt','Include assertions in the code',1),
    BoolOption('debug','Build with debug options turned on',1),
    BoolOption('inline','Cause inline code to be inline',0),
    BoolOption('optimize','Build object files with optimization',0),
    BoolOption('profile','Generate profiling aware code',0),
    BoolOption('small','Generate smaller code rather than faster',0),
    BoolOption('config','Generation the configuration data',0)
  )
  opts.Update(env)
  env['CC']       = 'gcc'
  env['CCFLAGS']  = '-pipe -Wall -Wcast-align -Wpointer-arith'
  env['CXXFLAGS'] = join([
    "-pipe -Wall -Wcast-align -Wpointer-arith -Wno-deprecated -Wold-style-cast",
    "-Woverloaded-virtual -ffor-scope -fno-operator-names"])
  env['CPPDEFINES'] = { '__STDC_LIMIT_MACROS':None }
  env.Prepend(CPPPATH='#')
  VariantName=''
  if env['small'] == 1:
    VariantName='S'
    env.Append(CCFLAGS='-Os')
    env.Append(CXXFLAGS='-Os')
  else :
    VariantName='s'

  if env['profile'] == 1:
    VariantName+='P'
    env.Append(CCFLAGS='-pg')
    env.Append(CXXFLAGS='-pg')
  else :
    VariantName+='p'

  if env['assrt'] == 1:
    VariantName+='A'
    env.Append(CPPDEFINES={'HLVM_ASSERT':None})
  else :
    VariantName+='a'

  if env['debug'] == 1 :
    VariantName += 'D'
    env.Append(CCFLAGS='-g')
    env.Append(CXXFLAGS='-g')
    env.Append(CPPDEFINES={'HLVM_DEBUG':None})
  else :
    VariantName+='d'

  if env['inline'] == 1:
    VariantName+='I'
  else :
    VariantName+='i'
    env.Append(CXXFLAGS='-fno-inline')

  if env['optimize'] == 1 :
    VariantName+='O'
    env.APpend(CCFLAGS='-O3')
    env.Append(CXXFLAGS='-O3')
  else :
    VariantName+='o'
    env.Append(CCFLAGS='-O1')
    env.Append(CXXFLAGS='-O1')

  env['Variant'] = VariantName
  env['BuildDir'] = 'build.'
  env['BuildDir'] += VariantName

  opts.Save('options.cache', env)
  env.Help(opts.GenerateHelpText(env,sort=cmp))
  return env;
