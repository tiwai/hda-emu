#!/usr/bin/env python

# hda-emu-tester - a test framework around hda-emu
#
# Written by David Henningsson <david.henningsson@canonical.com>
#
#    Copyright 2012 Canonical Ltd.
#
#    This program is free software: you can redistribute it and/or modify it 
#    under the terms of the GNU General Public License version 3, as published 
#    by the Free Software Foundation.
#
#    This program is distributed in the hope that it will be useful, but 
#    WITHOUT ANY WARRANTY; without even the implied warranties of 
#    MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR 
#    PURPOSE.  See the GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License along 
#    with this program.  If not, see <http://www.gnu.org/licenses/>.

def handle_argv():
    import argparse
    parser = argparse.ArgumentParser(description='Hda-emu automated test wrapper.')
    parser.add_argument('--file', dest='file', required=True, help='alsa-info filename')
    parser.add_argument('--print-errors', dest='print_errors', action="store_true", 
        default=False, help='print errors, and the command that caused it')
    parser.add_argument('--comm-log', dest='comm_log', action="store_true", 
        default=False, help='print communication with hda-emu')
    return parser.parse_args()

def run_test(argv):
    import runner
    result = runner.HdaEmuRunner()
    result.set_print_errors(argv.print_errors)
    result.set_comm_log_enabled(argv.comm_log)
    result.set_alsa_info_file(argv.file)
    result.run_standard()
    return result

def get_exit_code(result):
    if result.errors > 9:
        return 29
    if result.errors > 0:
        return 20 + result.errors
    if result.warnings > 9:
        return 19
    if result.warnings > 0:
        return 10 + result.warnings
    return 0

def main():
    import sys
    try:
        argv_dict = handle_argv()
        result = run_test(argv_dict)
        exitcode = get_exit_code(result)
        if result.errors > 0 or result.warnings > 0:
            print '{0} errors, {1} warnings. ({2})'.format(result.errors, result.warnings, argv_dict.file)
    except:
        import traceback
        traceback.print_exc()
        sys.exit(99)

    sys.exit(exitcode)

if __name__ == "__main__":
    main()
