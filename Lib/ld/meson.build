#
# Includes
#
ld_path = meson.current_source_dir()


#
# Linker Args
#
add_project_link_arguments(
    '-L' + ld_path,
    language: 'c',
)


#
# MCU Info
#
info_mk20dx128vlf5 = {
    'cpuhz': 48000000,
    'flash': {
        'bootloader': 8192,
        'firmware': 122880,
    },
    'ld': {
        'bootloader': files(
            'mk.ld',
            'mk20dx128vlf5.bootloader.ld',
        ),
        'firmware': files(
            'mk.ld',
            'mk20dx128vlf5.ld',
        ),
    },
    'ram': 16384,
    'shortname': 'mk20dx128',
}

info_mk20dx256vlh7 = {
    'cpuhz': 72000000,
    'flash': {
        'bootloader': 8192,
        'firmware': 253952,
    },
    'ld': {
        'bootloader': files(
            'mk.ld',
            'mk20dx256vlh7.bootloader.ld',
        ),
        'firmware': files(
            'mk.ld',
            'mk20dx256vlh7.ld',
        ),
    },
    'ram': 65536,
    'shortname': 'mk20dx256',
}

info_sam4s2a = {
    'cpuhz': 120000000,
    'flash': {
        'bootloader': 16384,
        'firmware': 106496, # 16kB bootloader + 8kB nvstorage (131072 bytes)
    },
    'ld': {
        'bootloader': files(
            'sam4s.ld',
            'sam4s2a.bootloader.ld',
        ),
        'firmware': files(
            'sam4s.ld',
            'sam4s2a.ld',
        ),
    },
    'ram': 65536,
    'shortname': 'sam4s2',
}

info_sam4s4b = {
    'cpuhz': 120000000,
    'flash': {
        'bootloader': 16384,
        'firmware': 237568, # 16kB bootloader + 8kB nvstorage (262144 bytes)
    },
    'ld': {
        'bootloader': files(
            'sam4s.ld',
            'sam4s4b.bootloader.ld',
        ),
        'firmware': files(
            'sam4s.ld',
            'sam4s4b.ld',
        ),
    },
    'ram': 65536,
    'shortname': 'sam4s4',
}

info_sam4s8b = {
    'cpuhz': 120000000,
    'flash': {
        'bootloader': 16384,
        'firmware': 499712, # 16kB bootloader + 8kB nvstorage (524288 bytes)
    },
    'ld': {
        'bootloader': files(
            'sam4s.ld',
            'sam4s8b.bootloader.ld',
        ),
        'firmware': files(
            'sam4s.ld',
            'sam4s8b.ld',
        ),
    },
    'ram': 131072,
    'shortname': 'sam4s8',
}

info_sam4s8c = {
    'cpuhz': 120000000,
    'flash': {
        'bootloader': 16384,
        'firmware': 499712, # 16kB bootloader + 8kB nvstorage (524288 bytes)
    },
    'ld': {
        'bootloader': files(
            'sam4s.ld',
            'sam4s8c.bootloader.ld',
        ),
        'firmware': files(
            'sam4s.ld',
            'sam4s8c.ld',
        ),
    },
    'ram': 131072,
    'shortname': 'sam4s8',
}



# Bootloaders
ld_bootloader_mk20dx128vlf5 = files()

ld_bootloader_mk20dx128vlh7 = files(
    'mk.ld',
    'mk20dx128vlh7.bootloader.ld',
)

ld_bootloader_mk20dx256vlh7 = files(
    'mk.ld',
    'mk20dx256vlh7.bootloader.ld',
)

ld_bootloader_mk22fx512avlh12 = files(
    'mk.ld',
    'mk22fx512avlh12.bootloader.ld',
)

# Firmware
ld_mk20dx128_teensy = files(
    'mk.ld',
    'mk20dx128.ld',
)

ld_mk20dx128vlh7 = files(
    'mk.ld',
    'mk20dx128vlh7.ld',
)

ld_mk20dx256_teensy = files(
    'mk.ld',
    'mk20dx256.ld',
)

ld_mk22fx512avlh12 = files(
    'mk.ld',
    'mk22fx512avlh12.ld',
)

ld_mk22fx512avlh12 = files(
    'mk.ld',
    'mk22fx512avlh12.ld',
)

ld_mk64fx512_teensy = files(
    'mk.ld',
    'mk64fx512.ld',
)

ld_mk66fx1m0_teensy = files(
    'mk.ld',
    'mk66fx1m0.ld',
)

ld_sam4s2b = files(
    'sam4s.ld',
    'sam4s2b.ld',
)

ld_sam4s4a = files(
    'sam4s.ld',
    'sam4s4a.ld',
)

ld_sam4s4c = files(
    'sam4s.ld',
    'sam4s4c.ld',
)

ld_sam4s16b = files(
    'sam4s.ld',
    'sam4s16b.ld',
)

ld_sam4s16c = files(
    'sam4s.ld',
    'sam4s16c.ld',
)

ld_sam4sa16b = files(
    'sam4s.ld',
    'sam4sa16b.ld',
)

ld_sam4sa16c = files(
    'sam4s.ld',
    'sam4sa16c.ld',
)

ld_sam4sd16b = files(
    'sam4s.ld',
    'sam4sd16b.ld',
)

ld_sam4sd16c = files(
    'sam4s.ld',
    'sam4sd16c.ld',
)

ld_sam4sd32b = files(
    'sam4s.ld',
    'sam4sd32b.ld',
)

ld_sam4sd32c = files(
    'sam4s.ld',
    'sam4sd32c.ld',
)
