# -*- coding: utf-8 -*-
import os, sys

unittestflags = (['@run_all', '--unittest-xml']
                 if os.environ.get('BROKEN_CUSTOM_ARGS_UNITTESTS') else [])

name = 'scene_rdl2'

if 'early' not in locals() or not callable(early):
    def early(): return lambda x: x

@early()
def version():
    """
    Increment the build in the version.
    """
    _version = '12.9'
    from rezbuild import earlybind
    return earlybind.version(this, _version)

description = 'scene_rdl2 package'

authors = [
    'PSW Rendering and Shading',
    'moonbase-dev@dreamworks.com'
]

help = ('For assistance, '
        "please contact the folio's owner at: moonbase-dev@dreamworks.com")

if 'scons' in sys.argv:
    build_system = 'scons'
    build_system_pbr = 'bart_scons-10'
else:
    build_system = 'cmake'
    build_system_pbr = 'cmake_modules'

variants = [
    ['os-CentOS-7', 'opt_level-optdebug', 'refplat-vfx2021.0', 'gcc-9.3.x.1', 'python-3.7'],
    ['os-CentOS-7', 'opt_level-debug', 'refplat-vfx2021.0', 'gcc-9.3.x.1', 'python-3.7'],
    ['os-CentOS-7', 'opt_level-optdebug', 'refplat-vfx2021.0', 'clang-13', 'python-3.7'],

    ['os-CentOS-7', 'opt_level-optdebug', 'refplat-vfx2022.0', 'gcc-9.3.x.1', 'python-3.9'],
    ['os-CentOS-7', 'opt_level-debug', 'refplat-vfx2022.0', 'gcc-9.3.x.1', 'python-3.9'],

    ['os-CentOS-7', 'opt_level-optdebug', 'refplat-vfx2020.3', 'icc-19.0.5.281.x.2', 'python-3.7'],
    ['os-CentOS-7', 'opt_level-debug', 'refplat-vfx2020.3', 'icc-19.0.5.281.x.2', 'python-3.7'],
    ['os-CentOS-7', 'opt_level-optdebug', 'refplat-vfx2020.3', 'gcc-6.3.x.2', 'python-3.7'],
    ['os-CentOS-7', 'opt_level-debug', 'refplat-vfx2020.3', 'gcc-6.3.x.2', 'python-3.7'],

    ['os-CentOS-7', 'opt_level-optdebug', 'refplat-vfx2020.3', 'icc-19.0.5.281.x.2', 'python-2.7'],
    ['os-CentOS-7', 'opt_level-debug', 'refplat-vfx2020.3', 'icc-19.0.5.281.x.2', 'python-2.7'],
    ['os-CentOS-7', 'opt_level-optdebug', 'refplat-vfx2020.3', 'gcc-6.3.x.2', 'python-2.7'],
    ['os-CentOS-7', 'opt_level-debug', 'refplat-vfx2020.3', 'gcc-6.3.x.2', 'python-2.7'],

    ['os-CentOS-7', 'opt_level-optdebug', 'refplat-vfx2021.0', 'gcc-9.3.x.1', 'python-2.7'],
    ['os-CentOS-7', 'opt_level-debug', 'refplat-vfx2021.0', 'gcc-9.3.x.1', 'python-2.7'],

    ['os-CentOS-7', 'opt_level-optdebug', 'refplat-vfx2022.0', 'gcc-9.3.x.1', 'python-3.7'],
    ['os-CentOS-7', 'opt_level-debug', 'refplat-vfx2022.0', 'gcc-9.3.x.1', 'python-3.7'],

    ['os-CentOS-7', 'opt_level-optdebug', 'refplat-vfx2019.3', 'gcc-6.3.x.2', 'python-2.7'],
    ['os-CentOS-7', 'opt_level-debug', 'refplat-vfx2019.3', 'gcc-6.3.x.2', 'python-2.7'],

    ['os-rocky-9', 'opt_level-optdebug', 'refplat-vfx2021.0', 'gcc-9.3.x.1', 'python-3.7'],
    ['os-rocky-9', 'opt_level-debug', 'refplat-vfx2021.0', 'gcc-9.3.x.1', 'python-3.7'],
    ['os-rocky-9', 'opt_level-optdebug', 'refplat-vfx2022.0', 'gcc-9.3.x.1', 'python-3.9'],
    ['os-rocky-9', 'opt_level-debug', 'refplat-vfx2022.0', 'gcc-9.3.x.1', 'python-3.9'],
    ['os-rocky-9', 'opt_level-optdebug', 'refplat-vfx2021.0', 'gcc-9.3.x.1', 'python-2.7'],
    ['os-rocky-9', 'opt_level-debug', 'refplat-vfx2021.0', 'gcc-9.3.x.1', 'python-2.7'],
]

conf_rats_variants = variants[0:2]
conf_CI_variants = list(filter(lambda v: 'os-CentOS-7' in v, variants))

scons_targets = ['@install'] + unittestflags
sconsTargets = {
    'refplat-vfx2019.3': scons_targets,
    'refplat-vfx2020.3': scons_targets,
    'refplat-vfx2021.0': scons_targets,
    'refplat-vfx2022.0': scons_targets,
}

requires = [
    'boost',
    'jsoncpp-0.6.0',
    'log4cplus-1.1.2.x',
    'lua',
    'tbb',
]

private_build_requires = [
    build_system_pbr,
    'cppunit',
    'ispc-1.20.0.x',
]

commandstr = lambda i: "cd build/"+os.path.join(*variants[i])+"; ctest -j $(nproc)"
testentry = lambda i: ("variant%d" % i,
                       { "command": commandstr(i),
                         "requires": ["cmake-3.23"] + variants[i] } )
testlist = [testentry(i) for i in range(len(variants))]
tests = dict(testlist)

if build_system is 'cmake':
    def commands():
        prependenv('CMAKE_MODULE_PATH', '{root}/lib64/cmake')
        prependenv('CMAKE_PREFIX_PATH', '{root}')
        prependenv('LD_LIBRARY_PATH', '{root}/lib64')
        prependenv('PATH', '{root}/bin')
        prependenv('PYTHONPATH', '{root}/python/lib/$PYTHON_NAME')
else:
    def commands():
        prependenv('LD_LIBRARY_PATH', '{root}/lib')
        prependenv('PATH', '{root}/bin')
        prependenv('PYTHONPATH', '{root}/python/lib/$PYTHON_NAME')

uuid = 'b5f6c7f8-19d3-11e5-9ac2-2c27d7efd8c7'

config_version = 0
