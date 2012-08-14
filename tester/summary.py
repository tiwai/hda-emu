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

def main():
    import os
    import os.path
    import runner

    os.chdir(os.path.dirname(os.path.realpath(__file__)))
    directory = "../codecs/canonical/"
    files = os.listdir(directory)

    successes = 0
    fails = 0
    warnings = 0
    errors = 0
    for f in files:
        try:
            r = runner.HdaEmuRunner()
            r.set_alsa_info_file(os.path.join(directory, f))
            r.run_standard()
            if r.errors > 0 or r.warnings > 0:
                fails += 1
                errors += r.errors
                warnings += r.warnings
            else:
                successes += 1
        except:
            errors += 1
            fails += 1

    print "Test run of %d machines: %d failing with %d errors and %d warnings." % (successes + fails, fails, errors, warnings)

if __name__ == "__main__":
    main()
