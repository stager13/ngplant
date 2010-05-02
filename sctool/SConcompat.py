
# starting from 0.96.93 Clone() should be used, but for compatibility with
# older scons in case of error we will try to use Copy()
def EnvClone(env):
    try:
        return env.Clone()
    except:
        return env.Copy()

