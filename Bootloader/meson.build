#
# Sources
#
c_bootloader = files(
    'debug.c',
    'dfu.c',
    'dfu.desc.c',
    'flash.c',
    'main.c',
    'usb.c',
)

c_bootloader_kinetis = files(
    'kinetis.c',
)

c_bootloader_atsam = files(
    '../Lib/ASF/common/services/clock/sam4s/sysclk.c',
    '../Lib/ASF/common/services/isp/flip/sam/isp.c',
    '../Lib/ASF/common/services/usb/class/dfu_flip/device/udi_dfu_atmel.c',
    '../Lib/ASF/common/services/usb/class/dfu_flip/device/udi_dfu_atmel_desc.c',
    '../Lib/ASF/common/services/usb/udc/udc.c',
    '../Lib/ASF/common/utils/interrupt/interrupt_sam_nvic.c',
    '../Lib/ASF/sam/drivers/efc/efc.c',
    '../Lib/ASF/sam/drivers/matrix/matrix.c',
    '../Lib/ASF/sam/drivers/pmc/pmc.c',
    '../Lib/ASF/sam/drivers/udp/udp_device.c',
    '../Lib/ASF/sam/drivers/wdt/wdt.c',
    '../Lib/ASF/sam/services/flash_efc/flash_efc.c',
    '../Lib/ASF/sam/utils/cmsis/sam4s/source/templates/system_sam4s.c',
)


#
# Targets
#
bootloader_config = configuration_data({
    'BOOT_DFU_ALTNAME': 'Kiibohd DFU|Secure', # The | symbol is replaced by a space if in secure mode, or a \0 if not (at runtime)
    'BOOT_PRODUCT_ID': '0x01CB',
    'BOOT_PRODUCT_STR': 'Kiibohd DFU Bootloader',
    'BOOT_VENDOR_ID': '0x1209',
    'CHIP': '<Unknown>',
    'Git_Commit_Number': git_commit_number,
    'FLASHING_STATION_ID': 'xXXx', # TODO get_option
    'MANUFACTURER': 'Kiibohd'
})

c_bootloader_buildvars = configure_file(
    configuration: bootloader_config,
    input: '_buildvars.h',
    output: 'buildvars.h',
)

c_bootloader_output = [
    '@BASENAME@.hex',
    '@BASENAME@.bin',
    '@BASENAME@.lss',
    '@BASENAME@.sym',
]

bootloader_60v2 = custom_target(
    '60v2.bootloader',
    command: [
        finalize_bin,
        '--max-flash', info_sam4s8c['flash']['bootloader'].to_string(),
        '--max-ram', info_sam4s8c['ram'].to_string(),
    ],
    input: executable(
        '60v2.bootloader',
        [
            c_bootloader,
            c_bootloader_atsam,
            c_bootloader_buildvars,
            files(
                'Devices/sam4s8c.c',
                'Devices/60v2.c',
            ),
        ],
        include_directories: [
            top_include,
            asf_include,
        ],
        name_suffix: 'elf',
        c_args: [
            '-DF_CPU=@0@'.format(info_sam4s8c['cpuhz']),
            '-D__SAM4S8C__', # TODO Why?
            '-D_sam4s8c_=1',
            '-D_bootloader_',
            '-fplan9-extensions',
            '-DBOARD=SAM4S_XPLAINED_PRO', # TODO Is this needed?
            '-msoft-float',
            '-ggdb3',
            '-nostdlib',
        ],
        link_args: [
            #'-fwhole-program',  # TODO Is this needed?
            '-T../@0@'.format(info_sam4s8c['ld']['bootloader'][1]),
            '-fwhole-program',
            '-Wl,-Map=60v2.map',
            '-msoft-float',
            '-mthumb',
            '-nostdlib',
            '-nostartfiles',
        ],
        link_depends: [
            info_sam4s8c['ld']['bootloader'],
        ],
    ),
    output: c_bootloader_output,
    build_by_default: true,
)
