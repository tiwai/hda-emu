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

import subprocess
import os

class HdaEmuRunner():

    def __init__(self):
        self.child_args = "../hda-emu -M -F"
        self.alsa_info = "/proc/asound/card0/codec#0"
        self.errors = 0
        self.warnings = 0

    def set_alsa_info_file(self, filename):
        self.alsa_info = filename

    def start_process(self):
        import shlex
        from subprocess import PIPE, STDOUT

        args = shlex.split(self.child_args)
        args.append(self.alsa_info)
        self.child = subprocess.Popen(args, bufsize=0, stdin=PIPE, stdout=PIPE, stderr=PIPE)

    def stop_process(self):
        if self.child is None:
            return
        self.child.terminate()
        self.child.wait()
        self.child = None


    def check_stdout(self):
        s = os.read(self.child.stdout.fileno(), 65536)
        # print "Stdout received (", len(s), ")"
        if s == "":
            print "Unexpected EOF of stdout (hda-emu crashed?)"
            self.errors += 1
            return
        self.stdout_total += s

        q, _, self.stdout_total = self.stdout_total.rpartition('\n')
        for s in q.splitlines():
            if s.startswith("Error:"):
                self.errors += 1
                # print s
            if s.startswith("Warning:"):
                self.warnings += 1
                # print s

    def check_stderr(self):
        s = os.read(self.child.stderr.fileno(), 65536)
        # print "Stderr received (", len(s), ")"
        if s == "":
            print "Unexpected EOF of stderr (hda-emu crashed?)"
            self.errors += 1
            return False
        if s == "> ":
            return True
        print "Unexpected stderr output: '" + prompt + "'"
        self.errors += 1
        return False

    def run_command(self, command=None):
        if command:
            self.child.stdin.write(command + '\n')
            self.child.stdin.flush()

        import select
        self.stdout_total = ""
        pipelist = [self.child.stdout, self.child.stderr]
        while True:
            readable, _, broken = select.select(pipelist, [], pipelist, 3)
            for q in broken:
                print "Broken pipe (hda-emu crashed?)"
                self.errors += 1
                return
            if readable == []:
                print "Timeout waiting for hda-emu"
                self.errors += 1
                return
            if self.child.stdout in readable:
                self.check_stdout()
            if self.child.stderr in readable:
                if self.check_stderr():
                    return

    def run_standard(self):
        self.start_process()
        self.run_command() # Initial parsing
        self.run_command("pm") # S3 test
        self.stop_process()

