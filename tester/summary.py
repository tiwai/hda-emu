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

import runner


def defaultpath(s):
    import os.path
    q = os.path.dirname(os.path.realpath(__file__))
    return os.path.join(q, s)

def count_codecs(filename):
    r = runner.HdaEmuRunner()
    r.set_alsa_info_file(filename)
    r.set_codec_index(99999)
    r.start_process()
    try:
        r.run_command()
    except runner.HdaEmuFatalError:
        import re
        m = re.compile("Codec index (\d+) requested, but found only (\d+) codecs")
        for line in r.error_list:
            match = m.search(line)
            if match:
                return int(match.group(2))
    return -1

def main():
    import os
    import os.path

    import argparse
    parser = argparse.ArgumentParser(description='Hda-emu automated test wrapper.')
    parser.add_argument('--verbose', '-v', action='count')
    parser.add_argument('--directory', '-d', action='store', default=defaultpath("../codecs/canonical/"))
    parser.add_argument('--recursive', '-r', action="store_true", default=False)

    parser_dict = parser.parse_args()
    verbose = parser_dict.verbose
    directory = parser_dict.directory

    if parser_dict.recursive:
        files = [os.path.join(root, filename) for root, _, filenames in os.walk(directory) for filename in filenames]
    else:
        files = os.listdir(directory)
    files.sort()

    successes = 0
    fails = 0
    warnings = 0
    errors = 0
    index = 0
    for f in files:
        index += 1
        if verbose > 2:
            print '[{0}/{1}]: Testing {2}'.format(index, len(files), f)

        try:
            codecs = count_codecs(os.path.join(directory, f))

            fail_this = False
            skip_this = codecs < 0
            for codec_index in range(codecs):
                r = runner.HdaEmuRunner()
                r.set_alsa_info_file(os.path.join(directory, f))
                r.set_codec_index(codec_index)
                r.set_print_errors(verbose > 1)

                try:
                    r.run_standard()
                except runner.HdaEmuFatalError:
                    for line in r.error_list:
                        if line.find("is a modem codec, aborting"):
                            skip_this = True

                if (not skip_this) and (r.errors > 0 or r.warnings > 0):
                    fail_this = True
                    errors += r.errors
                    warnings += r.warnings
                    if verbose > 0:
                        print '{0} errors, {1} warnings for {2}, codec {3}/{4}.'.format(r.errors, r.warnings, f, codec_index+1, codecs)

            if not skip_this:
                if fail_this:
                    fails += 1
                else:
                    successes += 1
        except KeyboardInterrupt:
            import sys
            sys.exit(1)
        except Exception as e:
            if verbose > 0:
                print 'Fatal error for {0}: {1}'.format(f, e)
            errors += 1
            fails += 1

    print "Test run of %d machines: %d failing with %d errors and %d warnings." % (successes + fails, fails, errors, warnings)

if __name__ == "__main__":
    main()
