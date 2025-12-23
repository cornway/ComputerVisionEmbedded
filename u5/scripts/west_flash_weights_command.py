# Copyright (c) 2025 STMicroelectronics
# All rights reserved.
#
# This software is licensed under terms that can be found in the LICENSE file
# in the root directory of this software component.
# If no LICENSE file comes with this software, it is provided AS-IS.

from west.commands import WestCommand

import os
from pathlib import Path
import platform
import shutil
import subprocess

DEFAULT_FILENAME = "app/Model/network_data.hex"

class FlashWeights(WestCommand):
    def __init__(self):
        super().__init__(
            'flash-weights', # => self.name
            'flash model weights', # => self.help
            # => self.description:
            '''Flash model weights''')

    def do_add_parser(self, parser_adder):
        parser = parser_adder.add_parser(self.name,
                                         help=self.help,
                                         description=self.description)
        parser.add_argument('filename', nargs='?', default=DEFAULT_FILENAME, help="default if not present (%s)" % DEFAULT_FILENAME)

        return parser

    def do_run(self, args, unknown_args):
      cli = FlashWeights._get_stm32cubeprogrammer_path()
      extloader = FlashWeights._get_stm32cubeprogrammer_path().parent.resolve() / 'ExternalLoader' / 'MX66UW1G45G_STM32N6570-DK.stldr'
      cmd = [str(cli)]
      cmd += ['-c', 'port=SWD', 'mode=HOTPLUG'] #connect
      cmd += ['-el', str(extloader)] #eternal loader
      cmd += ['-w', args.filename]

      subprocess.check_call(cmd)

    @staticmethod
    def _get_stm32cubeprogrammer_path() -> Path:
        """Obtain path of the STM32CubeProgrammer CLI tool."""

        if platform.system() == "Linux":
            cmd = shutil.which("STM32_Programmer_CLI")
            if cmd is not None:
                return Path(cmd)

            return (
                Path.home()
                / "STMicroelectronics"
                / "STM32Cube"
                / "STM32CubeProgrammer"
                / "bin"
                / "STM32_Programmer_CLI"
            )

        if platform.system() == "Windows":
            cmd = shutil.which("STM32_Programmer_CLI")
            if cmd is not None:
                return Path(cmd)

            cli = (
                Path("STMicroelectronics")
                / "STM32Cube"
                / "STM32CubeProgrammer"
                / "bin"
                / "STM32_Programmer_CLI.exe"
            )
            x86_path = Path(os.environ["PROGRAMFILES(X86)"]) / cli
            if x86_path.exists():
                return x86_path

            return Path(os.environ["PROGRAMW6432"]) / cli

        if platform.system() == "Darwin":
            return (
                Path("/Applications")
                / "STMicroelectronics"
                / "STM32Cube"
                / "STM32CubeProgrammer"
                / "STM32CubeProgrammer.app"
                / "Contents"
                / "MacOs"
                / "bin"
                / "STM32_Programmer_CLI"
            )

        raise NotImplementedError("Could not determine STM32_Programmer_CLI path")

