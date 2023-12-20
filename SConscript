Import('env')

env.Tool('component')
env.Tool('dwa_install')
env.Tool('dwa_run_test')
env.Tool('dwa_utils')
env.Tool('python_sdk')

#TODO: resolve cppcheck 1.71 errors, remove this line
env['CPPCHECK_LOC'] = '/rel/third_party/cppcheck/1.62/cppcheck'
env.Tool('cppcheck')

env.Tool('scene_rdl2_utils', toolpath = ['bart_tools'])
env.Tool('rdl2_dso', toolpath = ['bart_tools'])

env.DefaultToAVX()

from dwa_sdk import DWALoadSDKs
DWALoadSDKs(env)

import datetime

# ARRAS specific flags
DWAAddOption('--localized',
            dest='localized',
            action='store_true',
            help="Used to bring in ARRAS dependencies for release builds"
            )

DWAAddOption('--add-yaml',
            dest='add-yaml',
            action='store_true',
            help="Used to install package.yaml when building without rez."
            )

# Suppress depication warning from tbb-2020.
env.AppendUnique(CPPDEFINES='TBB_SUPPRESS_DEPRECATED_MESSAGES')

# For SBB integration - SBB uses the keyword "restrict", so the compiler needs to enable it.
if 'icc' in env['CC']:
    env['CXXFLAGS'].append('-restrict')
env.Replace(USE_OPENMP = [])

#Set optimization level for debug -O0
#icc defaults to -O2 for debug and -O3 for opt/opt-debug
if env['TYPE_LABEL'] == 'debug':
    env.AppendUnique(CCFLAGS = ['-O0'])
    if '-inline-level=0' not in env['CXXFLAGS'] and 'icc' in env['CC']:
        env['CXXFLAGS'].append('-inline-level=0')

# don't error on writes to static vars
if 'icc' in env['CC'] and '-we1711' in env['CXXFLAGS']:
    env['CXXFLAGS'].remove('-we1711')

# For Arras, we've made the decision to part with the studio standards and use #pragma once
# instead of include guards. We disable warning 1782 to avoid the related (incorrect) compiler spew.
if 'icc' in env['CC'] and '-wd1782' not in env['CXXFLAGS']:
    env['CXXFLAGS'].append('-wd1782') 		# #pragma once is obsolete. Use #ifndef guard instead.

#Preloads tbbmalloc_proxy & tbbmalloc for faster memory allocation
# if not --disable_tbb_malloc_preload, Enable TBB malloc preloading
if env['TYPE_LABEL'] != 'debug':
    if env['COMPILER_LABEL'] not in ['gccHoudini165_64', 'gccHoudini160_64', 'iccvfx2016_64', 'iccMaya2018_64']:
        env.PreloadTBBMalloc()

# Compiler flags
if 'icc' in env['CC']:
    # Warnings in RDL code.
    env['CXXFLAGS'].append('-wd1711') # Unfortunately we need static variables.

    env['CXXFLAGS'].append('-wd1684') # W: Conversion from pointer to same-sized integral type
    env['CXXFLAGS'].append('-wd2203') # W: Cast discards qualifiers from target type
    env['CXXFLAGS'].append('-wd869')  # W: Parameter was never referenced (frontend bug with variadic templates)


elif 'gcc' in env['CC']:
    # GCC - CXXFLAGS
    env['CXXFLAGS'].append('-Wno-narrowing')       # W: Conversion int to short int may alter value
    env['CXXFLAGS'].append('-Wno-conversion')      # W: Conversion between datatypes may alter value
    env['CXXFLAGS'].append('-Wno-switch')          # W: Not all possible cases are explicitly handled
    env['CXXFLAGS'].append('-Wno-unknown-pragmas') # W: Don't yell at me about pragmas
    env['CXXFLAGS'].append('-Wno-unused-variable') # W: Don't yell at me about unused variables
    env['CXXFLAGS'].append('-Wno-sign-compare')    # W: Sign vs. Unsigned comparisons
    env['CXXFLAGS'].append('-Wno-unused-local-typedefs') # W: Locally defined typedef but not used
    env['CXXFLAGS'].append('-Wno-class-memaccess') # W: with no trivial copy-assignment
    env['CXXFLAGS'].append('-Wno-maybe-uninitialized') # W: may be used uninitialized in this function
    env['CXXFLAGS'].append('-Wno-cast-function-type') # W: cast between incompatible function types
    env['CXXFLAGS'].append('-fno-var-tracking-assignments') # W: Turn off variable tracking
    env['CXXFLAGS'].append('-mavx')  # O: Enable AVX
    env['CXXFLAGS'].append('-mfma') # O: Enable FMA
    env['CXXFLAGS'].append('-ffast-math') # O: Fast Math
    env['CXXFLAGS'].append('-msse')

    # GCC - CPPDEFINES
    env['CPPDEFINES'].append(('__cdecl', '""'))
    env['CPPDEFINES'].append(('"__pragma(p)"', '""'))
elif 'clang' in env['CC']:
    #TODO: Taking a break with clang. It's kind of a PITA to debug. I'm going to re-evaluate my understanding of it and come back to it later
    env['CXXFLAGS'].append('-Wno-c++98-compat')          # W: Some things are not supported by C++98 (variadic templates)
    env['CXXFLAGS'].append('-Wno-c++98-compat-pedantic') # W: They REALLY don't like variadic macros
    env['CXXFLAGS'].append('-Wno-missing-prototypes')    # W: No previous prototype for functions
    env['CXXFLAGS'].append('-Wno-conversion')            # W: Conversion between datatypes may alter value
    env['CXXFLAGS'].append('-Wno-weak-vtables')          # W: 'GUIDException' has no out-of-line virtual method definitions (Are we ok with this?)
    env['CXXFLAGS'].append('-Wno-reserved-id-macro')     # W: Macro name is a reserved identifier (Are we ok with this?)
    env['CXXFLAGS'].append('-Wno-deprecated')            # W: Dynamic exception specifications are deprecated
    env['CXXFLAGS'].append('-Wno-padded')                # W: Padding size of 'variable' with x bytes to alignment boundary - Supposedly normal
    env['CXXFLAGS'].append('-Wno-missing-variable-declarations') # No previous extern declaration for non-static variable
    env['CXXFLAGS'].append('-Wno-variadic-macros')       # W: Named variadic macros are a GNU extension
    env['CXXFLAGS'].append('-Wno-extra-semi')            # W: Extra semi-colon after member function definition (really?)
    env['CXXFLAGS'].append('-Wno-old-style-cast')        # W: Use of old-style cast (This could be fixed, but does it need to be?)
    env['CXXFLAGS'].append('-Wno-gnu-zero-variadic-macro-arguments') # Ok.
    env['CXXFLAGS'].append('-Wno-sign-compare')          # W: Sign vs. Unsigned comparisons
    env['CXXFLAGS'].append('-Wno-global-constructors')   # W: Declaration requires a global constructors (but I don't think it does)
    env['CXXFLAGS'].append('-Wno-undefined-reinterpret-cast') # Dereference of type has undefined behavior (const int * -> const float *)
    env['CXXFLAGS'].append('-Wno-cast-align')            # W: Increases required alignment for types
    env['CXXFLAGS'].append('-Wno-unused-macros')         # W: Macro is not used
    env['CXXFLAGS'].append('-Wno-narrowing')             # W: Narrowing to a smaller datatype
    env['CXXFLAGS'].append('-Wno-c++11-narrowing')       # W: Ok maybe this one too
    env['CXXFLAGS'].append('-Wno-shadow')                # W: Declaration shadows a field
    env['CXXFLAGS'].append('-Wno-float-equal')           # W: Comparing floating point with == or != is unsafe
    env['CXXFLAGS'].append('-Wno-double-promotion')      # W: Implicit conversion from float to double
    env['CXXFLAGS'].append('-Wno-unknown-pragmas')       # W: Yup...
    env['CXXFLAGS'].append('-Wno-undef')                 # W: Not defined, evaluates to 0 (that's what we want right?)
    env['CXXFLAGS'].append('-Wno-gnu-anonymous-struct')  # W: Anonymous structs are a GNU extension
    env['CXXFLAGS'].append('-Wno-nested-anon-types')     # W: Anonymous types declared in an anonymous union are an extension
    env['CXXFLAGS'].append('-Wno-c++14-extensions')      # W: Use of "throw" in a constexpr function is a C++14 extension
    env['CXXFLAGS'].append('-Wno-switch')                # W: Not all possible cases are explicitly handled
    env['CXXFLAGS'].append('-Wno-switch-enum')           # W: Gotta do this one too.

    # clang - CPPDEFINES
    env['CPPDEFINES'].append(('__cdecl', '""'))
    env['CPPDEFINES'].append(('"__pragma(p)"', '""'))

env.TreatAllWarningsAsErrors()

env.DWASConscriptWalk(topdir='#cmd')

env.DWASConscriptWalk(topdir='#mod', ignore=[])

env.DWASConscriptWalk(topdir='#lib')

env.DWASConscriptWalk(topdir='#tests')

env.DWAInstallSDKScript('#SDKScript')

env.NoCache(env.DWAInstallFiles(
        'bart', ['#bart_tools/%s.py' % name for name in ('scene_rdl2_utils', 'rdl2_dso')]))

if GetOption('add-yaml'):
    env.DWAInstallRezPackage('$INSTALL_DIR')

env.DWAResolveUndefinedComponents([])
env.DWAFreezeComponents()

# Set default target
env.Default(env.Alias('@install'))

# @cppcheck target
env.DWACppcheck(env.Dir('.'), [
        "bart_tools",
        "boot",
        "site_tools",
        "build",
        "build_env",
        "install",
        ".git",
        "data",
        "doc",
        ])

env.Alias('@run_all')

def get_copyright_year(starting_year):
    current_year = datetime.date.today().year
    if starting_year == current_year:
        return str(starting_year)
    else:
        assert starting_year < current_year
        return str(starting_year) + '-' + str(current_year)

# Get the version number string that rez provide padding it out with .0.0.0 in case it does not have enough fields, must have a
# major version to work. Split the version string up on '.'s taking the first four and converting them to ints. Zip them together with
# their corresponding label and use that to create a dictionary that we will use later to expand the label values in the version file
# template. Add an entry in the dictionary for the version string as an integer.
version_parts = map(int, (env['BUILDINFO']['REZ_BUILD_PROJECT_VERSION']+'.0.0.0').split('.')[0:4])
part_names = ['@major@', '@minor@', '@patch@', '@build@']
version_dict = dict(zip(part_names, version_parts))
version_dict['@number@'] = int('{}{:04}{:04}{:04}'.format(*version_parts))
version_dict['@copyrightyear@'] = get_copyright_year(2022)

# Create a template for the version file's contents with label strings for the version info we wish to substitute in.
version_tmpl = '''// Copyright @copyrightyear@ DreamWorks Animation LLC
// SPDX-License-Identifier: Apache-2.0

#pragma once
#define RDL2_VERSION_STRING "@major@.@minor@.@patch@.@build@"

#define RDL2_VERSION_MAJOR @major@
#define RDL2_VERSION_MINOR @minor@
#define RDL2_VERSION_PATCH @patch@
#define RDL2_VERSION_BUILD @build@

#define RDL2_VERSION_NUMBER @number@

'''

# Generate the version file from the template substituting in version values from the version_dict.
version_h = env.Textfile(
target='version.h',
source=version_tmpl,
SUBST_DICT=version_dict,
TEXTFILESUFFIX='')

# Install the version file into the packages' include directory tree with the second parameter being the path in the include tree
# to install it to which in this case would be .../include/scene_rdl2/version.h
env.DWAInstallInclude([version_h], 'scene_rdl2')

# I need to disable cppcheck as part of the unittests, it doesn't like
# the definition of DWA_ASSERT.  It produces a slew
# of messages of the form:
# "(warning) Redundant code: Found a statement that begins with numeric constant."
# when DWA_ASSERT is defined as static_cast<void>(0).  I don't understand why
# that same exact definition works fine in dwa/Assert.h
# env.Alias('@run_all', env.Alias('@cppcheck'))
