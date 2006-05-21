from SCons.Environment import Environment as Environment
from environment import ProvisionEnvironment as ProvisionEnvironment
def GetBuildEnvironment(targets):
  env = Environment();
  env = ProvisionEnvironment(env)
  if env['config'] == 1:
    from configure import ConfigureHLVM as ConfigureHLVM
    env = ConfigureHLVM(env)
  return env
