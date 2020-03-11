#!/usr/bin/env python3
'''
Finalize elf file into an MCU compatible binary.

Also computes and generates some useful extra information.
'''
# Jacob Alexander 2020

import argparse
import os
import subprocess


def show_usage(title, used, total):
    '''
    Calculates the percentage used, then displays to stdout
    '''
    percentage = used * 100 / total

    # Change color based on how close to total is used
    color = '\t\033[1;32m'
    if percentage > 95:
        color = '\t\033[1;5;31m'
    elif percentage > 90:
        color = '\t\033[1;31m'
    elif percentage > 50:
        color = '\t\033[1;33m'

    print('\t\033[1m{title}\033[m:{color}{percentage:0.1f}%\033[m \t{used}/{total}\tbytes'.format(
        title=title,
        color=color,
        percentage=percentage,
        used=used,
        total=total,
    ))


if __name__ == '__main__':
    # Collect arguments
    parser = argparse.ArgumentParser(
        description="Generates hex, bin, lss, and sym from an elf executable",
    )

    parser.add_argument(
        'elf',
        action='store',
        help='Path to a file containing elf symbols',
    )

    parser.add_argument(
        '--out-dir',
        action='store',
        default=os.getcwd(),
        help='Directory used to generate files',
    )

    parser.add_argument(
        '--nm',
        action='store',
        help='Path to nm binary.',
    )

    parser.add_argument(
        '--objcopy',
        action='store',
        help='Path to objcopy binary.',
    )

    parser.add_argument(
        '--objdump',
        action='store',
        help='Path to objdump binary.',
    )

    parser.add_argument(
        '--size',
        action='store',
        help='Path to size binary.',
    )

    parser.add_argument(
        '--max-flash',
        action='store',
        help='Max flash size, in bytes.',
    )

    parser.add_argument(
        '--max-ram',
        action='store',
        help='Max ram size, in bytes.',
    )

    args = parser.parse_args()

    # Gather required args
    elf = args.elf
    name = os.path.splitext(os.path.basename(elf))[0]
    outdir = args.out_dir

    # sym file
    if args.nm:
        result = subprocess.run(
            [args.nm, '-n', elf],
            check=True,
            capture_output=True,
        )
        sym_file = os.path.join(outdir, '{}.sym'.format(name))
        with open(sym_file, 'wb') as ofile:
            ofile.write(result.stdout)

    # bin and hex files
    if args.objcopy:
        bin_file = os.path.join(outdir, '{}.bin'.format(name))
        subprocess.run(
            [args.objcopy, '-O', 'binary', elf, bin_file],
            check=True,
        )

        hex_file = os.path.join(outdir, '{}.hex'.format(name))
        subprocess.run(
            [args.objcopy, '-O', 'ihex', elf, hex_file],
            check=True,
        )

    # lss file
    if args.objdump:
        # TODO Check for clang and use these args
        # -section-headers --triple=arm-none-eabi
        result = subprocess.run(
            [args.objdump, '-h', '-S', '-z', elf],
            check=True,
            capture_output=True,
        )
        lss_file = os.path.join(outdir, '{}.lss'.format(name))
        with open(lss_file, 'wb') as ofile:
            ofile.write(result.stdout)

    # Calculate size information
    if args.size:
        size_info = subprocess.run(
            [args.size, elf],
            check=True,
            capture_output=True,
        )
        fields = size_info.stdout.split(b'\n')[1].split()
        text = int(fields[0])
        data = int(fields[1])
        bss = int(fields[2])

        # Compute flash and ram usage (NOTE: This will not compute stack usage, that requires runtime analysis)
        flash = text + data
        ram = data + bss

        # Check max sram
        if args.max_ram:
            show_usage('SRAM', ram, int(args.max_ram))

        # Check max flash
        if args.max_flash:
            show_usage('Flash', flash, int(args.max_flash))
