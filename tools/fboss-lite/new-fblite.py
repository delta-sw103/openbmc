#!/usr/local/bin/python3
#
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This program file is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; version 2 of the License.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program in a file named COPYING; if not, write to the
# Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor,
# Boston, MA 02110-1301 USA

#
# This script auto-generates the machine layer for new BMC-Lite FBOSS
# OpenBMC platforms.
#
# It's done by copying "tools/fboss-lite/fblite-ref-layer" reference layer
# to "meta-facebook/meta-<machine>" and replacing the predefined keywords
# with proper values (such as "@FBMODEL@" --> <machine-name>).
#
# Below are the global configurations in the reference layer, and we need
# to update the reference layer when something is changed:
#   BMC SoC                : AST26xx
#   U-boot Version         : v2019.04
#   Kernel Version         : 6.0.%
#   Init Manager           : systemd
#   Flash data0 Filesystem : UBIFS
#   eMMC filesystem        : EXT4
#   BMC Console            : uart1 (refer to aspeed-g6.dtsi)
#   mTerm Console          : uart5 (refer aspeed-g6.dtsi)
#   BMC MAC Controller     : mac3 (refer aspeed-g6.dtsi)
#   BMC to OOB-Switch      : fixed-link at 1Gbps
#

#
# TODO:
#   1. select desired yocto version in "openbmc-init-build-env" automatically.
#   2. auto-create Chassis and SCM EEPROM in device tree or setup_i2c.sh.
#   3. auto-generate setup-gpio logic if needed.
#   4. auto-generate "pwrcpld" driver based on its register map.
#   5. auto-generate power control logic if item #4 is automated.
#   6. auto-generate recovery path (depending on BMC-Lite System Reference
#      design)
#

import argparse
import os
import shutil
import subprocess
import sys


#
# Global files and directories.
#
OBMC_BUILD_ENV_FILE = "openbmc-init-build-env"
FBLITE_REF_LAYER = "tools/fboss-lite/fblite-ref-layer"
OBMC_META_FB = "meta-facebook"

#
# Predefined keywords in reference layer, and need to be updated when
# producing machine layer code.
#
KEY_MODEL_NAME = "@FBMODEL@"

#
# Global variables
#
TMP_DIR = "/tmp"
COMMIT_TEMPLATE = "tools/fboss-lite/commit-template.txt"


def run_shell_cmd(cmd):
    f = subprocess.Popen(
        cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE
    )
    data, err = f.communicate()
    info = data.decode("utf-8")
    return info


def setup_exec_env():
    """Setup running environment."""
    # Set work directory to the root of openbmc repo
    cli_path = os.path.abspath(sys.argv[0])
    cli_dir = os.path.dirname(cli_path)
    os.chdir(os.path.join(cli_dir, "../.."))
    print("Work directory is set to %s" % os.getcwd())

    # Sanity test if we are at the root of openbmc repo
    for name in [OBMC_BUILD_ENV_FILE, FBLITE_REF_LAYER]:
        if not os.path.exists(name):
            print("Error: %s doesn't exist!" % name)
            sys.exit(1)


def update_machine_code(pathname, name):
    """Replace @KEY@ with proper values in machine layer file."""
    cmd = "sed -i -e 's/%s/%s/g' %s" % (KEY_MODEL_NAME, name, pathname)
    run_shell_cmd(cmd)


def commit_machine_layer(name, machine_layer):
    """Commit the machine layer code to local repo."""
    commit_file = os.path.join(TMP_DIR, "tmp_commit.txt")

    try:
        # Prepare commit message
        shutil.copyfile(COMMIT_TEMPLATE, commit_file)
        cmd = "sed -i -e 's/%s/%s/g' %s" % (KEY_MODEL_NAME, name, commit_file)
        run_shell_cmd(cmd)

        print("Commit the patch to local repo..")
        run_shell_cmd("git add -f %s" % machine_layer)
        run_shell_cmd("git commit -F %s" % commit_file)
    finally:
        os.remove(commit_file)


if __name__ == "__main__":
    """Create a new fboss-lite machine layer."""
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "-n", "--name", type=str, required=True, help="model name of the new platform"
    )
    args = parser.parse_args()

    #
    # Set up running environment
    #
    setup_exec_env()

    #
    # Copy reference layer to the machine layer
    #
    machine_layer = os.path.join(OBMC_META_FB, "meta-%s" % args.name)
    if os.path.exists(machine_layer):
        print("Error: %s was already created. Exiting!" % machine_layer)
        sys.exit(1)
    print("Copy %s to %s.." % (FBLITE_REF_LAYER, machine_layer))
    shutil.copytree(FBLITE_REF_LAYER, machine_layer)

    #
    # Update files with proper values in the machine layer
    #
    for root, _dirs, files in os.walk(machine_layer):
        if not files:
            continue

        print("processing files under %s.." % root)
        for f_entry in files:
            pathname = os.path.join(root, f_entry)
            update_machine_code(pathname, args.name)

            # Rename leaf files if needed
            if KEY_MODEL_NAME in f_entry:
                new_name = f_entry.replace(KEY_MODEL_NAME, args.name)
                os.rename(pathname, os.path.join(root, new_name))

    #
    # Commit the patch in local tree.
    #
    commit_machine_layer(args.name, machine_layer)
