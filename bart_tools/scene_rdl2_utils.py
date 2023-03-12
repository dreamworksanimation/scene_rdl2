# Copyright 2023 DreamWorks Animation LLC
# SPDX-License-Identifier: Apache-2.0

'''
A general collection of DWA-specific SCons wrappers.
'''
from SCons.Script.Main import GetOption, AddOption
from SCons.Tool import ProgramScanner
from site_init import DWAAddOption
from SCons.Script.SConscript import SConsEnvironment
from SCons.Script import Variables
from ansi_colors import colorize
import atexit
import sys, re, os, errno
from subprocess import check_call
import SCons.Defaults
from SCons.Action import Action
import subprocess
import yaml

def DefaultToAVX(env):
    # Set AVX as architecture if not architecture flag is passed
    if not GetOption('target-architecture'):
        # Using core-avx-i rather than ivybridge even for newer compilers because
        # using ivybridge breaks moonray.
        default_march = 'core-avx2'
        AddOption('--arch-bypass',
                dest='target-architecture',
                action='store',
                type='string',
                default=default_march)
        try:
            env['CXXFLAGS'].remove('-march=corei7')
            env.AppendUnique(CXXFLAGS='-march=%s' % default_march)
        except:
            pass

def TreatAllWarningsAsErrors(env):
    # Set warnings to errors unless --no_Werrors was passed.
    if GetOption('allow_warnings'):
        print colorize('* NOTE: Warnings will NOT be treated as errors.\n', 'red')
    elif 'icc' in env['CC']:
        env.AppendUnique(CXXFLAGS = '-Werror-all')

def CompilerCheckAndExit(self, expected_compiler=None):
    # Check if env compiler is expected compiler, it not print help and exit.
    if (GetOption('skip_default_compiler_check') or
        expected_compiler is None or
        self['COMPILER_LABEL']==expected_compiler):
        pass
    else:
        print colorize(
        '* ERROR\n'
        '* The current compiler (%s) differs from the default compiler for ARRAS %s \n' %
                (self['COMPILER_LABEL'], expected_compiler), 'red')
        check_call("scons -h --skip_default_compiler_check",shell=True)
        sys.exit(1)

def EnvCleanUp(env):
    # Override the default environment inherited from bart set up
    # Note: This could also go into the top-level-SConscript

    #starting with BaRT-1.7-59+ gcc libpath was hardcoded in bart to
    #help premo and other projects in finding gcc libs
    #is incompatible with code built using the RHEL6 default GCC
    pass

def PreloadTBBMalloc(env):
    # Preloads TBB MALLOC libraries for faster memory allocation in some cases.
    if GetOption('disable_tbb_malloc_preload'):
        if env['TYPE_LABEL'] != 'debug':
            print colorize('******** WARNING****************\n'
                           '* TBB malloc disabled*\n'
                           '**********************************************',
                           'yellow')
    else:
        libs = ['tbbmalloc_proxy', 'tbbmalloc']
        try:
            from lookup import getBackingVersion, getTBBFragment, getTBBPath, \
                getTBBVersion
            TBBLibPath = (
                getTBBPath(getTBBVersion(env['COMPILER_LABEL'])) +
                getTBBFragment(getBackingVersion(env['COMPILER_LABEL'])))
        except ImportError:
            try:
                relpath = 'lib/intel64/gcc4.4' if int(os.environ[
                        'REZ_TBB_MAJOR_VERSION']) == 4 else 'lib'
                TBBLibPath = os.path.join(os.environ['REZ_TBB_ROOT'], relpath)
            except KeyError:
                TBBLibPath = env.subst('$TBB_DIR/lib/intel64/gcc4.4')
        env.Prepend(LIBS=libs, LIBPATH=[TBBLibPath])

def AddExtraCPPDEFINES(env):
    # Add extra defines to CPPDEFINES
    if GetOption('extra_defines'):
        for extra_cppdefine in set(GetOption('extra_defines').split(',')):
            env.Append(CPPDEFINES = [extra_cppdefine])

def InstallAnimation(env):
    if not GetOption('install_animation'):
        if GetOption('verbose') or GetOption('verbose-human'):
            print colorize('* NOTICE \n'
                           '* Ignoring animation related tools in build/install. \n'
                           '* Use --install_animation to build these tools. \n',
                           'blue')

def runCmd(cmd, blocking=True, quiet=False, quiet_errors=False):
    """
    Abstraction for shelling out to run commands

    if not blocking
        continue execution in non-zero return code cases

    if quiet:
        block stdout out

    if quiet_errors:
        hides stderr

    """
    p = subprocess.Popen(
        args=cmd,
        shell=True,
        stderr=subprocess.PIPE,
        stdout=subprocess.PIPE
    )
    stdout, stderr = p.communicate()

    if not quiet:
        if stdout:
            sys.stdout.write(stdout)
    if not quiet_errors:
        if stderr:
            sys.stderr.write(stderr)
    else:
        if blocking:
            raise Exception('%s failed\n' % cmd)

    return p.returncode, stdout, stderr

def _localizeAction(target, source, env):

    target = target[0].abspath
    LDD_CMD = "%s %s" % ("ldd", target)
    output = runCmd(LDD_CMD, quiet=True, quiet_errors=True, blocking=False)
    result = output[1]
    lines = result.split("\n")
    libs = {}
    for line in lines:
        match = re.match(r'\t(.*) => (/(rel|work)/.*) \(0x', line)
        if match:
            libs[match.group(1)] = match.group(2)

    if libs:
        install_path = os.path.join(env.Dir(env['INSTALL_DIR']).abspath, 'ext')
        try:
            os.makedirs(install_path)
        except OSError as exc:
            if exc.errno == errno.EEXIST and os.path.isdir(install_path):
                pass
        else:
            raise

        for lib in libs:
            RSYNC_CMD = 'rsync  --chmod a=rx %s %s/%s' % (libs[lib], install_path , lib)
            runCmd(RSYNC_CMD, quiet=True, quiet_errors=False, blocking=False)

def strfunc(target, source, env):
    LOCALIZECOMSTR = ''
    if GetOption('verbose') or GetOption('verbose-human'):
        LOCALIZECOMSTR = colorize('[rsync]', 'purple') + '%s=>ext/%s' % (target[0].name, target[0].name)
    return LOCALIZECOMSTR

def CopyExternDeps(env):

    """Register ext (relative to $INSTALL_DIR) so it does not get cleaned by DWAUninstallUnused"""
    env.DWARegisterUncontrolledInstallDir('ext')

    LocalizeAction = Action(_localizeAction, strfunc)

    """Overrides SharedLibrary "stock" Builder in an Environment. Appends additional action to
    Rsync external dependencies to local install dir """
    shared_lib_action_list = [ SCons.Defaults.SharedCheck,
                            SCons.Defaults.ShLinkAction,
                            LocalizeAction ]

    shared_lib = SCons.Builder.Builder(action = shared_lib_action_list,
                                       emitter = "$SHLIBEMITTER",
                                       prefix = '$SHLIBPREFIX',
                                       suffix = '$SHLIBSUFFIX',
                                       target_scanner = ProgramScanner,
                                       src_suffix = '$SHOBJSUFFIX',
                                       src_builder = 'SharedObject')
    env['BUILDERS']['SharedLibrary'] = shared_lib

    program_action_list = [ SCons.Defaults.LinkAction,
                    SCons.Defaults.ShLinkAction,
                    LocalizeAction]
    """Overrides Program "stock" Builder in an Environment. Appends additional action to
    Rsync external dependencies to local install dir """
    program = SCons.Builder.Builder(action = program_action_list,
                                    emitter = '$PROGEMITTER',
                                    prefix = '$PROGPREFIX',
                                    suffix = '$PROGSUFFIX',
                                    src_suffix = '$OBJSUFFIX',
                                    src_builder = 'Object',
                                    target_scanner = ProgramScanner)
    env['BUILDERS']['Program'] = program

# The following dictionary and two class methods aid in the copying of
# proxies and class description to a proxy package to reduce unneeded
# package dependencies for packages that only require those bits.
proxyLocations = {
    'JSON_CLASS_DESCRIPTIONS': ('coredata/*.json',),
    'SO_PROXIES': ('rdl2dso/*.so.proxy', 'rdl2dso.proxy/*.so.proxy')
    }

def GatherProxies(env):
    """
    Gather up the paths to any proxy or json class descriptions files in this
    package so that they can be copied to the proxy package.
    """

    proxies = {}
    for var, pats in proxyLocations.items():
        proxies[var] = sum(map(lambda(pat): env.Glob(pat), pats), [])
    env.AppendUnique(**proxies)

def CopyProxies(env):
    """
    Copy any proxies or json class descriptions previously gathered up and copy
    them to the correct location in the current package.
    """

    targets = []
    for var, pats in proxyLocations.items():
        t = env.DWAInstallFiles(os.path.dirname(pats[0]), env.get(var, []))
        targets.extend(t)
        env.NoCache(t)
    return targets

def generate(env):
    DWAAddOption('--allow_warnings',
              dest='allow_warnings',
              action='store_true',
              help='Do not treat warnings as errors')

    DWAAddOption('--skip_default_compiler_check',
              dest='skip_default_compiler_check',
              action='store_true',
              help='continue with the build even if compiler label does not match default compiler')

    DWAAddOption('--disable_tbb_malloc_preload',
              dest='disable_tbb_malloc_preload',
              action='store_true',
              help='disable preloading of tbb_malloc, \
              disabled by default for debug builds, other build types use: --disable_tbb_malloc_preload')

    DWAAddOption('--skip_rdl2dsoproxy',
              dest='skip_rdl2dsoproxy',
              action='store_true',
              help='skip install of .so.proxy files under rdl2dso dir, \
              tmp flag to help with moonray/raas workflow')

    DWAAddOption('--defines',
              dest='extra_defines',
              type='string',
              nargs=1,
              action='store',
              help='Additional cppdefines separated by commas. Do not include the -D.',
              default=None)

    DWAAddOption('--install_animation',
              dest='install_animation',
              action='store_true',
              help='Build and Install animation, behavior, perception related tools')

    DWAAddOption('--localized',
              dest='localized',
              action='store_true',
              help="Used to bring in ARRAS dependencies for release builds",
              default=False)

    # Don't load this tool more than once
    if env.has_key('SCENE_RDL2_UTILS_TOOL_LOADED'):
        return
    if GetOption('localized'):
        print colorize('* NOTE: Third Party Libraries and other dependencies will be copied locally in <install_dir>/ext \n', 'purple')
        CopyExternDeps(env)

    env.SetDefault(SCENE_RDL2_UTILS_TOOL_LOADED = True)
    env.AddMethod(TreatAllWarningsAsErrors)
    env.AddMethod(CompilerCheckAndExit)
    env.AddMethod(EnvCleanUp)
    env.AddMethod(PreloadTBBMalloc)
    env.AddMethod(AddExtraCPPDEFINES)
    env.AddMethod(InstallAnimation)
    env.AddMethod(DefaultToAVX)
    env.AddMethod(GatherProxies)
    env.AddMethod(CopyProxies)

def exists(env):
    return True

