project(
    'kiibohd controller',
    'c',
    default_options: [
        'c_std=gnu11',
        'b_lto=false',
        'optimization=s',
    ],
    license: ['lgplv3'],
    meson_version: '>= 0.53',
    version: '0.5.6',
)


#
# Options
#
#configurator = get_option('configurator') == ''


#
# Variables
#
c_compiler = meson.get_compiler('c')
python_bin = find_program('python3')
nm = find_program('nm')
objcopy = find_program('objcopy')
objdump = find_program('objdump')
size = find_program('size')
finalize_bin = [
    python_bin,
    meson.current_source_dir() + '/Lib/Meson/finalize.py',
    '--out-dir', '@OUTDIR@',
    '--nm', nm,
    '--objcopy', objcopy,
    '--objdump', objdump,
    '--size', size,
    '@INPUT@',
]

# Build Date
if build_machine.system() == 'windows'
    build_date = run_command(
        find_program('cmd'), '/c', 'echo %date%__%time%',
    ).stdout().strip()
else
    build_date = run_command(
        find_program('date'), '+%Y-%m-%d', '%T', '%z',
    ).stdout().strip()
endif

# Git Information
git_bin = find_program('git', version: '>= 1.8', required: true) # TODO Make git optional
if git_bin.found()
    # Modified
    git_modified_status = run_command(
        git_bin, 'status', '-s', '-uno', '--porcelain',
    ).stdout().strip().split('\n').length() == 0 ? 'Clean' : 'Dirty'

    # List of modified files (\r\n is needed for the debug terminal)
    git_modified_files = '\r\n'.join(
        run_command(
            git_bin, 'diff-index', '--name-only', 'HEAD', '--',
        ).stdout().strip().split('\n')
    )

    # Branch
    git_branch_info = run_command(
        git_bin, 'rev-parse', '--abbrev-ref', 'HEAD',
    ).stdout().strip()

    # Date
    git_date_info = run_command(
        git_bin, 'show', '-s', '--format=%ci',
    ).stdout().strip()

    # Commit Author and Email
    git_commit_author = run_command(
        git_bin, 'show', '-s', '--format="%cn <%ce>"',
    ).stdout().strip()

    # Commit Revision
    git_commit_revision = run_command(
        git_bin, 'show', '-s', '--format=%H',
    ).stdout().strip()

    # Commit Number (on branch)
    git_commit_number = run_command(
        git_bin, 'rev-list', '--count', 'HEAD',
    ).stdout().strip().to_int()

    # Most Recent Tag (on branch)
    git_recent_tag = run_command(
        git_bin, 'for-each-ref', 'refs/tags', '--sort=-taggerdate', '"--format=%(refname:short)"', '--count=1',
    ).stdout().strip().split('/tags/')[1]

    # Most Recent Tag Hash (on branch)
    git_recent_tag_revision = run_command(
        git_bin, 'rev-list', '-n', '1', git_recent_tag,
    ).stdout().strip()

    # If not tags are set, use HEAD
    if git_recent_tag == ''
        git_recent_tag_revision = git_commit_revision
    endif

    # Most Recent Tag Commit Number (on branch)
    git_recent_tag_commit_number = run_command(
        git_bin, 'rev-list', '--count', git_recent_tag_revision,
    ).stdout().strip()

    # Origin URL
    git_origin_url = run_command(
        git_bin, 'config', '--get', 'remote.origin.url',
    ).stdout().strip()

    # Last Commit Date
    git_last_commit_date = '@0@ @1@ - @2@'.format(
        git_modified_status,
        git_branch_info,
        git_date_info,
    )

    summary(
        {
            'Commit Number': git_commit_number,
            'Commit Hash': git_commit_revision,
            'Recent Tag': git_recent_tag,
            'Last Commit Date': git_last_commit_date,
        },
        section: 'Git',
    )
endif

# Build Platform
build_os = 'linux'
if build_machine.system() == 'linux'
    # Attempt to use lsb_release to determine Linux distro
    lsb_release_bin = find_program('lsb_release', required: false)
    if lsb_release_bin.found()
        build_os = run_command(
            lsb_release_bin, '-dcs',
        ).stdout().strip().split('"')[1]
    endif
endif


#
# Includes
#
asf_include = include_directories(
    'Lib/ASF',
    'Lib/ASF/config',
    'Lib/CMSIS/Include',
)
top_include = include_directories('.')


#
# KLL
#
# TODO Make wrap, and default if subproject is cloned
#kll = subproject('kll')
#kll_bin = kll.get_variable('kll')
kll_bin = find_program(
    'kll',
    version: '>= 0.5.7.15',
)
kll_version = run_command([kll_bin, '--version']).stdout().strip()
kll_default_layouts_path = run_command([kll_bin, '--path']).stdout().strip()
kll_layout_cache_path = run_command([kll_bin, '--layout-cache-path']).stdout().strip()


#
# KLL Arguments
#
kll_cmd = [
    kll_bin,
    '--kiibohd-debug',
    '--emitter kiibohd',
    '--def-output kll_defs.h',
    '--map-output generatedKeymap.h',
    '--hid-output usb_hid.h',
    '--pixel-output generatedPixelmap.c',
    '--json-output kll.json',
    '${kll_configurator_options}',
]

kll_arg_config = [
    '--config',
]

kll_arg_base = [
    '--base',
]

kll_arg_default = [
    '--default',
]

kll_arg_partial = [
    '--partial',
]

kll_arg_configurator = [
    '--preprocessor-tmp-path',
    meson.current_build_dir() + '/tmp_kll',
]

kll_arg_debug_options = [
    '--operation-organization-debug',
    '--data-organization-debug',
    '--data-finalization-debug',
    '--data-analysis-debug',
]

kll_arg_display_options = [
    '--operation-organization-display',
    '--data-organization-display',
    '--data-finalization-display',
    '--data-analysis-display',
]

kll_arg_final_display_options = [
    '--data-analysis-display',
]


#
# Subdirectories (order-dependent)
#
subdir('Lib')

subdir('Debug')
subdir('Macro')
#subdir('Scan')
#subdir('Output')

subdir('Bootloader')
#subdir('Keyboards')


#
# Summary
#
summary(
    {
        'Version': kll_version,
        'Path': kll_bin.path(),
        'Default Layouts': kll_default_layouts_path,
        'HID Mapping Cache': kll_layout_cache_path,
    },
    section: 'KLL',
)

summary(
    {
        'C Compiler': c_compiler.get_id() + ' ' + c_compiler.version() + ' ' + ' '.join(c_compiler.cmd_array()),
        'Linker': c_compiler.get_linker_id(),
        'Target Arch': target_machine.cpu(),
        'Native Arch': build_machine.cpu(),
        'Native OS': build_os,
    },
    section: 'Compiler Environment',
)

summary(
    {
        'Build Dir': meson.current_build_dir(),
    },
    section: 'Build Environment',
)
