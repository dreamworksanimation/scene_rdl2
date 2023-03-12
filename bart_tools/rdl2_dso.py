# Copyright 2023 DreamWorks Animation LLC
# SPDX-License-Identifier: Apache-2.0


'''
Methods specific to RDL2 DSOs.
'''

import dwa_install
import dwa_class_proto

from SCons.Action import Action
from SCons.Builder import Builder
from SCons.Scanner import Scanner
from SCons.Node.FS import File, Dir
from SCons.Util import CLVar
from SCons.Script.Main import GetOption

from ansi_colors import colorize
import sys
import os
import re

# Regular expression used in parsing a source file to find built-ins.  Currently used with lib/scene/rdl2/SceneContext.cc
built_in_classes_expr = re.compile("createBuiltInSceneClass\<(\w+)\>")

# ------------------------------------------------------------------------------

# Functors
class Rdl2XclsExporterBinFetcher(object):
    '''
    Dynamically determines the output path of the .xcls exporter built by the currently running SCons session
    Uses scene_rdl2 folio for binary path for environents other than scene_rdl2
    '''
    def __call__(self, target, source, env, for_signature):
        if 'SCENE_RDL2_DIR' in env:
            return env['SCENE_RDL2_DIR'] + '/bin/rdl2_xcls_exporter'
        elif 'SCENE_RDL2_AVX_DIR' in env:
            return env['SCENE_RDL2_AVX_DIR'] + '/bin/rdl2_xcls_exporter'
        else:
            return env.Alias("rdl2_xcls_exporter")[0].sources[1].path

class Rdl2JsonExporterBinFetcher(object):
    '''
    Dynamically determines the output path of the .json exporter built by the currently running SCons session
    Uses scene_rdl2 folio for binary path for environents other than scene_rdl2
    '''
    def __call__(self, target, source, env, for_signature):
        if 'SCENE_RDL2_DIR' in env:
            return env['SCENE_RDL2_DIR'] + '/bin/rdl2_json_exporter'
        elif 'SCENE_RDL2_AVX_DIR' in env:
            return env['SCENE_RDL2_AVX_DIR'] + '/bin/rdl2_json_exporter'
        elif len(env.Alias("rdl2_json_exporter")[0].sources) > 1:
            return env.Alias("rdl2_json_exporter")[0].sources[1].path
        else:
            return filter(os.path.exists,
                          map(lambda d: os.path.join(d, 'rdl2_json_exporter'),
                              os.environ.get('PATH').split(':')))

class Rdl2DsoPathExpander(object):
    '''
    Returns the absoluate path of the RDL2 DSO path used by the NXGADB exporters
    '''
    def __call__(self, target, source, env, for_signature):
        return source[0].dir.abspath


class Rdl2BuiltinClassNameFetcher(object):
    def __call__(self, target, source, env, for_signature):
        result = []
        for elm in target:
            result.append(elm.name[:-len(elm.suffix.lstr)])
        return "--in " + " --in ".join(result)


class Rdl2BuiltinClassTargetsFetcher(object):
    def __call__(self, target, source, env, for_signature):
        result = []
        for elm in target:
            result.append(elm.path)
        return "--out " + " --out ".join(result)

class Rdl2BuiltinGroupTargetsFetcher(object):
    def __call__(self, target, source, env, for_signature):
        result = []
        for elm in target:
            result.append(elm.path)
        return "--groupings " + " --groupings ".join(result)


# Emitters

# Loads the contents of SceneContext.cc and find all builtin classes that will be need to
# have .xcls files exported
def nxgadb_xcls_rdl2_builtin_emitter(target, source, env):
    '''
    This emitter parses the contents of the input source file and looks for any built-in RDL2 DSOS
    as specified by the regular expression.  For each, it creates a corresponding .xcls nxgadb class
    file target
    '''
    built_ins = built_in_classes_expr.findall(source[0].get_contents())
    targets = []
    for elm in built_ins:
        targets.append(env.File(elm + "$RDL2_XCLS_EXPORTER_SUFFIX"))

    env.Depends(targets, env.Alias('rdl2_xcls_exporter'))

    classes = env.Alias("@classes", targets)
    env.Alias("@install", classes)
    env.NoCache(targets)
    return targets, source

# Loads the contents of SceneContext.cc and find all builtin classes that will be need to
# have .grp files exported
def nxgadb_grp_rdl2_builtin_emitter(target, source, env):
    '''
    This emitter parses the contents of the input source file and looks for any built-in RDL2 DSOS
    as specified by the regular expression.  For each, it creates a corresponding .grp groupings
    file target
    '''
    built_ins = built_in_classes_expr.findall(source[0].get_contents())
    targets = []
    for elm in built_ins:
        targets.append(env.File(elm + "$RDL2_GRP_EXPORTER_SUFFIX"))

    env.Depends(targets, env.Alias('rdl2_xcls_exporter'))

    groupings = env.Alias("@groupings", targets)
    env.Alias("@install", groupings)
    env.NoCache(targets)
    return targets, source

# Loads the contents of SceneContext.cc and find all builtin classes that will be need to
# have .json files exported
def json_rdl2_builtin_emitter(target, source, env):
    '''
    This emitter parses the contents of the input source file and looks for any built-in RDL2 DSOS
    as specified by the regular expression.  For each, it creates a corresponding .json nxgadb class
    file target
    '''
    built_ins = built_in_classes_expr.findall(source[0].get_contents())
    targets = []
    for elm in built_ins:
        targets.append(env.File(elm + "$RDL2_JSON_EXPORTER_SUFFIX"))

    env.Depends(targets, env.Alias('rdl2_json_exporter'))

    classes = env.Alias("@classes", targets)
    env.Alias("@install", classes)
    env.NoCache(targets)
    return targets, source

# This emitter emits the '.xcls' files, and also sets up the Aliases
def nxgadb_xcls_emitter(target, source, env):
    '''
    This emitter just assigns the correct dependencies between the source .proxy.so files and the corresponding
    nxgadb .xcls files it emits
    '''
    classes = env.Alias("@classes", target)
    env.Alias("@install", classes)
    # Set the dependency between the proxy dso and the building of the rdl2 exporter
    env.Depends(target, env.Alias('rdl2_xcls_exporter'))
    env.NoCache(target)
    return target, source

# This emitter emits the '.grp' files, and also sets up the Aliases
def nxgadb_grp_emitter(target, source, env):
    '''
    This emitter just assigns the correct dependencies between the source .proxy.so files and the corresponding
    grpoupings .grp files it emits
    '''
    groupings = env.Alias("@groupings", target)
    env.Alias("@install", groupings)
    # Set the dependency between the proxy dso and the building of the rdl2 exporter
    env.Depends(target, env.Alias('rdl2_xcls_exporter'))
    env.NoCache(target)
    return target, source

# This emitter emits the '.json' files, and also sets up the Aliases
def json_emitter(target, source, env):
    '''
    This emitter just assigns the correct dependencies between the source .proxy.so files and the corresponding
    nxgadb .json files it emits
    '''
    classes = env.Alias("@classes", target)
    env.Alias("@install", classes)
    # Set the dependency between the proxy dso and the building of the rdl2 exporter
    env.Depends(target, env.Alias('rdl2_json_exporter'))
    env.NoCache(target)
    return target, source


# Environment Methods
def DWARdl2Dso(env, target, source, **kwargs):
    '''
    Wrapper around DWASharedLibrary for declaring RDL2 loadable DSOs.

    These DSOs are intended to be loaded using dlopen() and unloaded using
    dlclose(), and should not be linked directly to programs.

    This will build two copies of the DSO. A "full" DSO and a "proxy" DSO.
    The proxy DSOs have their components list stripped clean and only compile
    the attributes.cc source file. All other source files (but not including
    attributes.cc) are compiled into the full DSO. The full DSO should
    #include "attributes.cc" in its main class definition source file.
    '''
    # Clone the environment for our DSO.
    dsoEnv = env.Clone()

    # Adjust the target build directory if asked for. This is mainly only used
    # to put all the unit test DSOs in the same directory.
    if isinstance(target, basestring):
        target = dsoEnv.File(target)
    buildTarget = os.path.dirname(target.get_abspath())
    if 'RDL2_BUILD_PATH_APPEND' in kwargs:
        buildTarget = buildTarget + '/' + kwargs['RDL2_BUILD_PATH_APPEND']
    buildTarget = buildTarget + '/' + os.path.basename(target.get_abspath())
    target = dsoEnv.File(os.path.abspath(buildTarget))

    # Hide all symbols that aren't explicitly exported.
    dsoEnv.AppendUnique(CXXFLAGS = '-fvisibility=hidden')
    dsoEnv.AppendUnique(LINKFLAGS = '-fvisibility=hidden')

    # Clone the environment for the proxy DSO, clearing any existing
    # library linkage.
    proxyLinkFlags = str(dsoEnv['LINKFLAGS']).replace(
        "${COMPONENTS_EXPAND_VAR('LINKFLAGS')}", "")
    libs = [] if dsoEnv.get('DEPOT_VARIANT_LABEL') else [
        '${COMPONENTS_EXPAND_VAR(\'LIBS\')}']
    proxyEnv = dsoEnv.Clone(CC='gcc', CXX='g++', LIBS=libs, LIBS_WHOLE=[],
                            LIBS_GROUP=[], LINKFLAGS=CLVar(proxyLinkFlags))

    # Force the proxy to build with GCC, but need to remove ICC only flags first
    if 'gcc' in env['CC']:
        dsoEnv.AppendUnique(CCFLAGS=['-fno-var-tracking-assignments'])

    # Remove any leftover ICC only flags.
    for i in range(proxyEnv['CXXFLAGS'].count('-restrict')):
        proxyEnv['CXXFLAGS'].remove('-restrict')
    for i in range(proxyEnv['CCFLAGS'].count('noinline-debug-info')):
        proxyEnv['CCFLAGS'].remove('noinline-debug-info')
    for i in range(proxyEnv['CXXFLAGS'].count('-Wcheck')):
        proxyEnv['CXXFLAGS'].remove('-Wcheck')
    for i in range(proxyEnv['CXXFLAGS'].count('-Wp64')):
        proxyEnv['CXXFLAGS'].remove('-Wp64')
    for i in range(proxyEnv['CXXFLAGS'].count('-Qoption,cpp,--print_include_stack')):
        proxyEnv['CXXFLAGS'].remove('-Qoption,cpp,--print_include_stack')
    for i in range(proxyEnv['CXXFLAGS'].count('-w3')):
        proxyEnv['CXXFLAGS'].remove('-w3')
    for i in range(proxyEnv['CCFLAGS'].count('-ip')):
        proxyEnv['CCFLAGS'].remove('-ip')
    for i in range(proxyEnv['LINKFLAGS'].count('-no-ipo')):
        proxyEnv['LINKFLAGS'].remove('-no-ipo')
    for i in range(proxyEnv['CCFLAGS'].count('-debug')):
        proxyEnv['CCFLAGS'].remove('-debug')
    for i in range(proxyEnv['CXXFLAGS'].count('-Werror-all')):
        proxyEnv['CXXFLAGS'].remove('-Werror-all')
    for i in range(proxyEnv['CCFLAGS'].count('-Werror-all')):
        proxyEnv['CCFLAGS'].remove('-Werror-all')
    for i in range(proxyEnv['CCFLAGS'].count('-inline-level=0')):
        proxyEnv['CCFLAGS'].remove('-inline-level=0')
    proxyEnv.Replace(CXXFLAGS=filter(lambda f: not (str(f).startswith('-wd') or
                                                    str(f).startswith('-we') or
                                                    str(f).startswith('-ww') or
                                                    str(f).startswith('-gcc-name') or
                                                    str(f).startswith('-gxx-name')),
                                     proxyEnv['CXXFLAGS']))

    # Remove any leftover clang only cxxflags.
    remove_clang_only_cxxflags = [
        '-faligned-allocation',
        '-fcxx-exceptions',
        '-fdelayed-template-parsing',
        '-fforce-enable-int128',
        '-frelaxed-template-template-args',
        '-fdouble-square-bracket-attributes',
        '-Wno-c++14-extensions',
        '-Wno-nested-anon-types',
        '-Wno-gnu-anonymous-struct',
        '-Wno-c++11-narrowing',
        '-Wno-undefined-reinterpret-cast',
        '-Wno-global-constructors',
        '-Wno-gnu-zero-variadic-macro-arguments',
        '-Wno-missing-variable-declarations',
        '-Wno-reserved-id-macro',
        '-Wno-weak-vtables',
        '-Wno-c++98-compat-pedantic',
        '-Wno-c++98-compat',
        '-Wno-missing-prototypes'
        ]
    for remove_flag in remove_clang_only_cxxflags:
        for i in range(proxyEnv['CXXFLAGS'].count(remove_flag)):
            proxyEnv['CXXFLAGS'].remove(remove_flag)

    proxyEnv.AppendUnique(
        CPPDEFINES=[('__cdecl', '""'), ('"__pragma(p)"', '""')],
        CCFLAGS=['-fabi-version=6', '-Wno-conversion', '-Wno-narrowing',
                 '-Wno-sign-compare', '-Wno-switch', '-Wno-unknown-pragmas',
                 '-fno-var-tracking-assignments'])

    # Remove ICC header paths that can break GCC.
    iccRoot = os.environ.get('REZ_ICC_ROOT')
    iccLocation = '/rel/third_party/intelcompiler64'
    if iccRoot:
        proxyEnv.Replace(CPPPATH_THIRDPARTY=filter(
            lambda f: not (str(f).startswith(iccRoot) or
                           str(f).startswith(iccLocation)),
            proxyEnv['CPPPATH_THIRDPARTY']))
    try:
        del proxyEnv['ENV']['CPLUS_INCLUDE_PATH']
    except:
        pass

    # Proxy DSOs only have RDL2 as a component.
    proxyEnv.DWAUseComponents(['scene_rdl2'])

    # Proxy DSOs end with ".proxy".
    proxyTarget = proxyEnv.File(target.get_abspath() + '.proxy')

    # Build the proxy DSO from attributes.cc.
    attributesSource = 'attributes.cc'
    if 'RDL2_ATTRIBUTES_SOURCE' in kwargs:
        attributesSource = kwargs['RDL2_ATTRIBUTES_SOURCE']
    proxyDso = proxyEnv.DWASharedLibrary(proxyTarget, attributesSource, **kwargs)

    # Remove attributes.cc from the sources list if it exists.
    if type(source) is list:
        if 'attributes.cc' in source:
            source.remove('attributes.cc')

    # Build the full DSO.
    dso = dsoEnv.DWASharedLibrary(target, source, **kwargs)

    dsoinfo = {
        'dsotype': 'rdl2',
        'source': dso,
        'proxy': proxyDso,
    }

    # If this isn't a test dso, then build a .xcls
    if kwargs.get('TEST_DSO', False) == False:
        # xcls = proxyEnv.Rdl2ToNxgadbClass(proxyDso)
        # groupings = proxyEnv.Rdl2ToGroupings(proxyDso)
        # dsoinfo['xcls'] = xcls
        # dsoinfo['groupings'] = groupings
        json = proxyEnv.Rdl2ToJson(proxyDso)
        dsoinfo['json'] = json

    # Make the proxy DSO depend on the full DSO.
    dsoEnv.Depends(dso, proxyDso)

    return dsoinfo

def DWAInstallRdl2Dso(env, dsoinfo):
    '''
    Installs DSOs into 'rdl2dso' to avoid clashing with other DSOs.
    '''
    if dsoinfo['dsotype'] != 'rdl2':
        raise ValueError('Error: DWAInstallRdl2Dso called with wrong dsotype')

    # Install the DSO into rdl2dso.
    dwa_install.installTargetIntoDir(env, '@install_dso', 'rdl2dso',
                                     dsoinfo['source'])

    if not env.GetOption('skip_rdl2dsoproxy'):
        # Install the proxy DSO into rdl2dso.
        target = dwa_install.installTargetIntoDir(env, '@install_dso',
                                                  'rdl2dso', dsoinfo['proxy'])
        env.Alias('@rdl2proxy', target)

    # Install the .xcls file
    if 'xcls' in dsoinfo:
        dwa_class_proto.installRdlPrototypeFromXcls(env, dsoinfo['xcls'][0])

    # Install the groupings file
    if 'groupings' in dsoinfo:
        dwa_class_proto.installClassGroupingsFile(env, '@install_groupings',
                                                  'groupings',
                                                  dsoinfo['groupings'][0])

    # Install the json file
    if 'json' in dsoinfo and dsoinfo['json']:
        target = dwa_install.installTargetIntoDir(env, '@install_dso',
                                                  'coredata', dsoinfo['json'])
        env.Alias('@rdl2proxy', target)

def DWARdl2DsoToNxgadb(env, dsoinfo):
    '''
    Uses the rdl2_xcls_exporter Builder to generate xcls files from an input RDL2 DSO.
    '''
    if dsoinfo['dsotype'] != 'rdl2':
        raise ValueError('Error: DWAInstallRdl2Dso called with wrong dsotype')

    # Run the builder
    return env.Rdl2ToNxgadbClass(dsoinfo['proxy'])

def DWARdl2DsoToGroupings(env, dsoinfo):
    '''
    Uses the rdl2_xcls_exporter Builder to generate groupings files from an input RDL2 DSO.
    '''
    if dsoinfo['dsotype'] != 'rdl2':
        raise ValueError('Error: DWAInstallRdl2Dso called with wrong dsotype')

    # Run the builder
    return env.Rdl2ToGroupings(dsoinfo['proxy'])

def DWARdl2DsoToJson(env, dsoinfo):
    '''
    Uses the rdl2_json_exporter Builder to generate json files from an input RDL2 DSO.
    '''
    if dsoinfo['dsotype'] != 'rdl2':
        raise ValueError('Error: DWAInstallRdl2Dso called with wrong dsotype')

    # Run the builder
    return env.Rdl2ToJson(dsoinfo['proxy'])

def DWARdl2BuiltinToNxgadb(env, gen_source='SceneContext.cc'):
    '''
    Uses the rdl2_xcls_exporter Builder to generate xcls files for an RDL2 builtin
    NOTE: Currently hard-coded to use with lib/scene/rdl2/SceneContext.cc for searching for builtins
    '''
    return env.Rdl2BuiltinToNxgadbClass(env.File(gen_source))

def DWARdl2BuiltinToGroupings(env, gen_source='SceneContext.cc'):
    '''
    Uses the rdl2_xcls_exporter Builder to generate groupings files for an RDL2 builtin
    NOTE: Currently hard-coded to use with lib/scene/rdl2/SceneContext.cc for searching for builtins
    '''
    return env.Rdl2BuiltinToGroupings(env.File(gen_source))

def DWARdl2BuiltinToJson(env, gen_source='SceneContext.cc'):
    '''
    Uses the rdl2_json_exporter Builder to generate json files for an RDL2 builtin
    NOTE: Currently hard-coded to use with lib/scene/rdl2/SceneContext.cc for searching for builtins
    '''
    return env.Rdl2BuiltinToJson(env.File(gen_source))


def DWAInstallRdl2BuiltinNxgadb(env, xcls):
    '''
    Runs the RDL prototype installer for each builtin .xcls file generated in a prior build step
    '''
    result = []
    for elm in xcls:
        result.append(dwa_class_proto.installRdlPrototypeFromXcls(env, elm))
    return result

def DWAInstallRdl2BuiltinGroupings(env, xs):
    '''
    Runs the RDL groups installer for each builtin .grp file generated in a prior build step
    '''
    result = []
    for elm in xs:
        result.append(dwa_class_proto.installClassGroupingsFile(env, '@install_groupings', 'groupings', elm))
    return result

def DWAInstallRdl2BuiltinJson(env, json):
    '''
    Runs the RDL prototype installer for each builtin .json file generated in a prior build step
    '''
    result = []
    for elm in json:
        result.append(dwa_install.installTargetIntoDir(env, '@install_dso', 'coredata', elm))
    return result

# Generate
def generate(env):
    #tool loaded in top level for renderer but missing for other projects
    #loading it here to get around the import issue
    env.Tool('dwa_class_proto')
    if 'dwa_install' not in env['TOOLS']:
        raise Exception("Error: cannot load \"rdl2_dso\" tool: " \
                        "\"dwa_install\" tool must be loaded first.")

    # If running MIC-AVX512 (KNL), we must use the intelsde emulator when
    # calling rdl2_xcls_exporter.
    sde_wrapper = ''
    if GetOption('target-architecture'):
        if 'MIC-AVX512' in GetOption('target-architecture'):
            sde_wrapper = '/rel/third_party/intelsde/intelsde-7.49.0/sde -knl -- '


    env.SetDefault(
        RDL2_XCLS_EXPORTER_BIN                 = sde_wrapper + "${_RDL2_XCLS_EXPORTER_BIN_FETCHER()}",
        RDL2_JSON_EXPORTER_BIN                 = sde_wrapper + "${_RDL2_JSON_EXPORTER_BIN_FETCHER()}",
        RDL2_EXPORTER_ARGS                     = "--dso_path ${_RDL2_EXPORTER_DSO_PATH()}",
        _RDL2_XCLS_EXPORTER_BIN_FETCHER        = Rdl2XclsExporterBinFetcher,
        _RDL2_JSON_EXPORTER_BIN_FETCHER        = Rdl2JsonExporterBinFetcher,
        _RDL2_EXPORTER_DSO_PATH                = Rdl2DsoPathExpander,
        # exporter commands for class files
        RDL2_XCLS_EXPORTER_RDL2_BUILTIN_COM    = "${RDL2_XCLS_EXPORTER_BIN} ${RDL2_EXPORTER_ARGS} --builtin ${RDL2_EXPORTER_BUILTIN_SOURCE_IN} ${RDL2_EXPORTER_BUILTIN_TARGET_OUT}",
        RDL2_JSON_EXPORTER_RDL2_BUILTIN_COM    = "${RDL2_JSON_EXPORTER_BIN} ${RDL2_EXPORTER_ARGS} --builtin ${RDL2_EXPORTER_BUILTIN_SOURCE_IN} ${RDL2_EXPORTER_BUILTIN_TARGET_OUT}",
        RDL2_XCLS_EXPORTER_COM                 = "${RDL2_XCLS_EXPORTER_BIN} ${RDL2_EXPORTER_ARGS} --in $SOURCE --out $TARGET",
        RDL2_JSON_EXPORTER_COM                 = "${RDL2_JSON_EXPORTER_BIN} ${RDL2_EXPORTER_ARGS} --in $SOURCE --out $TARGET",
        # xcls exporter commands for grouping files
        RDL2_GRP_EXPORTER_RDL2_BUILTIN_COM     = "${RDL2_XCLS_EXPORTER_BIN} ${RDL2_EXPORTER_ARGS} --builtin ${RDL2_EXPORTER_BUILTIN_SOURCE_IN} ${RDL2_GRP_EXPORTER_BUILTIN_TARGET_OUT}",
        RDL2_GRP_EXPORTER_COM                  = "${RDL2_XCLS_EXPORTER_BIN} ${RDL2_EXPORTER_ARGS} --in $SOURCE --groupings $TARGET",
        # fetchers for class files
        _RDL2_EXPORTER_BUILTIN_SOURCES         = Rdl2BuiltinClassNameFetcher,
        _RDL2_EXPORTER_BUILTIN_TARGETS         = Rdl2BuiltinClassTargetsFetcher,
        # fetchers for groupings files
        _RDL2_GRP_EXPORTER_BUILTIN_TARGETS     = Rdl2BuiltinGroupTargetsFetcher,
        # Class file sources and targets
        RDL2_EXPORTER_BUILTIN_SOURCE_IN        = "${_RDL2_EXPORTER_BUILTIN_SOURCES()}",
        RDL2_EXPORTER_BUILTIN_TARGET_OUT       = "${_RDL2_EXPORTER_BUILTIN_TARGETS()}",
        # Groupings file targets (uses same source as class files)
        RDL2_GRP_EXPORTER_BUILTIN_TARGET_OUT   = "${_RDL2_GRP_EXPORTER_BUILTIN_TARGETS()}",

        RDL2_XCLS_EXPORTER_COMSTR              = "${RDL2_XCLS_EXPORTER_COM}",
        RDL2_JSON_EXPORTER_COMSTR              = "${RDL2_JSON_EXPORTER_COM}",
        RDL2_XCLS_EXPORTER_RDL2_BUILTIN_COMSTR = "${RDL2_XCLS_EXPORTER_RDL2_BUILTIN_COM}",
        RDL2_JSON_EXPORTER_RDL2_BUILTIN_COMSTR = "${RDL2_JSON_EXPORTER_RDL2_BUILTIN_COM}",

        RDL2_GRP_EXPORTER_COMSTR               = "${RDL2_GRP_EXPORTER_COM}",
        RDL2_GRP_EXPORTER_RDL2_BUILTIN_COMSTR  = "${RDL2_GRP_EXPORTER_RDL2_BUILTIN_COM}",
        # Class file source and target suffixes
        RDL2_EXPORTER_SRC_SUFFIX               = ".so.proxy",
        RDL2_XCLS_EXPORTER_SUFFIX              = ".xcls",
        # Groupings file suffix
        RDL2_GRP_EXPORTER_SUFFIX               = ".grp",
        # JSON file suffix
        RDL2_JSON_EXPORTER_SUFFIX              = ".json",

        _RDL2_XCLS_EXPORTER_BUILT_IN_COUNT     = 0,
    )

    # Define the SCons Builder for non-builtin RDL2 DSO2
    nxgadb_xcls_builder = Builder(
        action         = Action('$RDL2_XCLS_EXPORTER_COM',
                                '$RDL2_XCLS_EXPORTER_COMSTR'),
        emitter        = nxgadb_xcls_emitter,
        src_suffix     = "$RDL2_EXPORTER_SRC_SUFFIX",
        suffix         = "$RDL2_XCLS_EXPORTER_SUFFIX",
        single_source  = True,
        target_factory = File,
        source_factory = File,
    )

    # Define the SCons Builder for non-builtin RDL2 DSO2 to produce groupings files
    nxgadb_grp_builder = Builder(
        action         = Action('$RDL2_GRP_EXPORTER_COM',
                                '$RDL2_GRP_EXPORTER_COMSTR'),
        emitter        = nxgadb_grp_emitter,
        src_suffix     = "$RDL2_EXPORTER_SRC_SUFFIX",
        suffix         = "$RDL2_GRP_EXPORTER_SUFFIX",
        single_source  = True,
        target_factory = File,
        source_factory = File,
    )

    # Define the SCons Builder for the RDL2 builtin classes to produce groupings files
    nxgadb_rdl2_builtin_grp_builder = Builder(
        action         = Action('$RDL2_GRP_EXPORTER_RDL2_BUILTIN_COM',
                                '$RDL2_GRP_EXPORTER_RDL2_BUILTIN_COMSTR'),
        emitter        = nxgadb_grp_rdl2_builtin_emitter,
        suffix         = "$RDL2_GRP_EXPORTER_SUFFIX",
        target_factory = File,
        source_factory = File,
    )

    # Define the SCons Builder for the RDL2 builtin classes
    nxgadb_rdl2_builtin_xcls_builder = Builder(
        action         = Action('$RDL2_XCLS_EXPORTER_RDL2_BUILTIN_COM',
                                '$RDL2_XCLS_EXPORTER_RDL2_BUILTIN_COMSTR'),
        emitter        = nxgadb_xcls_rdl2_builtin_emitter,
        suffix         = "$RDL2_XCLS_EXPORTER_SUFFIX",
        target_factory = File,
        source_factory = File,
    )

    # Define the SCons Builder for non-builtin RDL2 DSO2 JSON
    json_builder = Builder(
        action         = Action('$RDL2_JSON_EXPORTER_COM',
                                '$RDL2_JSON_EXPORTER_COMSTR'),
        emitter        = json_emitter,
        src_suffix     = "$RDL2_EXPORTER_SRC_SUFFIX",
        prefix         = 'json/',
        suffix         = "$RDL2_JSON_EXPORTER_SUFFIX",
        single_source  = True,
        target_factory = File,
        source_factory = File,
    )

    # Define the SCons Builder for the RDL2 builtin classes JSON
    rdl2_builtin_json_builder = Builder(
        action         = Action('$RDL2_JSON_EXPORTER_RDL2_BUILTIN_COM',
                                '$RDL2_JSON_EXPORTER_RDL2_BUILTIN_COMSTR'),
        emitter        = json_rdl2_builtin_emitter,
        suffix         = "$RDL2_JSON_EXPORTER_SUFFIX",
        target_factory = File,
        source_factory = File,
    )

# Add the builder and set the command generator.
    env.Append(BUILDERS = {
                 'Rdl2ToNxgadbClass': nxgadb_xcls_builder,
                 'Rdl2BuiltinToNxgadbClass': nxgadb_rdl2_builtin_xcls_builder,
                 'Rdl2ToGroupings': nxgadb_grp_builder,
                 'Rdl2BuiltinToGroupings': nxgadb_rdl2_builtin_grp_builder,
                 'Rdl2ToJson': json_builder,
                 'Rdl2BuiltinToJson': rdl2_builtin_json_builder
                 }
              )

    # Pretty printing.
    if not (env.GetOption('verbose') or env.GetOption('verbose-human')):
        env.Replace(RDL2_XCLS_EXPORTER_COMSTR = colorize(
                '[rdl2 -> NxgADB class gen]', 'purple') + ' $TARGET')
        env.Replace(RDL2_XCLS_EXPORTER_RDL2_BUILTIN_COMSTR = colorize(
                '[rdl2 builtin -> NxgADB class gen]', 'purple') + ' $TARGETS')
        env.Replace(RDL2_GRP_EXPORTER_COMSTR = colorize(
                '[rdl2 -> NxgADB groupings gen]', 'purple') + ' $TARGETS')
        env.Replace(RDL2_GRP_EXPORTER_RDL2_BUILTIN_COMSTR = colorize(
                '[rdl2 builtin -> NxgADB groupings gen]', 'purple') + ' $TARGETS')
        env.Replace(RDL2_JSON_EXPORTER_COMSTR = colorize(
                '[rdl2 -> JSON gen]', 'purple') + ' $TARGET')
        env.Replace(RDL2_JSON_EXPORTER_RDL2_BUILTIN_COMSTR = colorize(
                '[rdl2 builtin -> JSON gen]', 'purple') + ' $TARGETS')
    else:
        env.Replace(RDL2_XCLS_EXPORTER_COMSTR = '${RDL2_XCLS_EXPORTER_COM}')
        env.Replace(RDL2_XCLS_EXPORTER_RDL2_BUILTIN_COMSTR =
                    '${RDL2_XCLS_EXPORTER_RDL2_BUILTIN_COM}')
        env.Replace(RDL2_GRP_EXPORTER_COMSTR = '${RDL2_GRP_EXPORTER_COM}')
        env.Replace(RDL2_GRP_EXPORTER_RDL2_BUILTIN_COMSTR =
                    '${RDL2_GRP_EXPORTER_RDL2_BUILTIN_COM}')
        env.Replace(RDL2_JSON_EXPORTER_COMSTR = '${RDL2_JSON_EXPORTER_COM}')
        env.Replace(RDL2_JSON_EXPORTER_RDL2_BUILTIN_COMSTR =
                    '${RDL2_JSON_EXPORTER_RDL2_BUILTIN_COM}')


    # Add all methods here
    env.AddMethod(DWARdl2Dso)
    env.AddMethod(DWAInstallRdl2Dso)
    env.AddMethod(DWARdl2DsoToNxgadb)
    env.AddMethod(DWARdl2DsoToGroupings)
    env.AddMethod(DWARdl2DsoToJson)
    env.AddMethod(DWARdl2BuiltinToNxgadb)
    env.AddMethod(DWARdl2BuiltinToGroupings)
    env.AddMethod(DWARdl2BuiltinToJson)
    env.AddMethod(DWAInstallRdl2BuiltinNxgadb)
    env.AddMethod(DWAInstallRdl2BuiltinGroupings)
    env.AddMethod(DWAInstallRdl2BuiltinJson)

    # Add aliases
    env.DWAAlias('@rdl2proxy',
                 desc="Build all .so.proxy libraries released in $INSTALL_DIR/rdl2dso")

def exists(env):
    # TODO: Check for the existence of the .xcls builder? Other constraints?
    return True

