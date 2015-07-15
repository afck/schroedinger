CCFLAGS = ['-O3', '-march=native', '-std=c++11', '-Wall', '-pedantic']

env = Environment(CCFLAGS=CCFLAGS)
env.ParseConfig('sdl2-config --libs --cflags')
env.Program('schr', ['src/main.cc', 'src/Wave.cc'])

test_program = env.Program('test', ['src/FieldTest.cc'],
  CCFLAGS=CCFLAGS,
  LIBS=['cppunit', 'stdc++'])
test_alias = Alias('test', [test_program], test_program[0].abspath)
AlwaysBuild(test_alias)
