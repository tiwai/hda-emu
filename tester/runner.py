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
import re

class ControlInfo():
    def __init__(self, runner, list_info):
        self.runner = runner
        carr = list_info.split(":")
        self.numid = carr[1]
        self.name = carr[2]

    def add_info(self, get_info):
        minmax_regex = re.compile("MIN/MAX: (\d+)/(\d+),\s+VAL:(( \\[\d+\\])+)")
        val_regex = re.compile(" \\[(\d+)\\]")

        if self.name.endswith("Volume") or self.name.endswith("Switch"):
            match = minmax_regex.search("".join(get_info))
            if match is None:
                self.runner.add_error("Could not find MIN/MAX values for '" + self.name + "' (" + "".join(get_info) + ")", "Error")
                return
            self.minval = match.group(1)
            self.maxval = match.group(2)
            cval = val_regex.split(match.group(3))
            self.values = [int(v) for v in cval if v != ""]

class HdaEmuRunner():

    def __init__(self):
        self.child_args = "../hda-emu -M -F"
        self.alsa_info = "/proc/asound/card0/codec#0"
        self.errors = 0
        self.warnings = 0
        self.print_errors = False
        self.comm_log_enabled = False
        self.last_command = None

    def set_alsa_info_file(self, filename):
        self.alsa_info = filename

    def set_print_errors(self, print_errors):
        self.print_errors = print_errors

    def set_comm_log_enabled(self, value):
        self.comm_log_enabled = value

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

    def add_error(self, message, severity):
        if severity == "Warning":
            self.warnings += 1
        else:
            self.errors += 1
        if self.print_errors:
            if not self.last_command_printed:
                if self.last_command:
                    print "Encountered while running '" + self.last_command + "':"
                else:
                    print "Encountered during initial parsing:"
                self.last_command_printed = True
            print "  ", message

    def check_stdout(self):
        s = os.read(self.child.stdout.fileno(), 65536)

        if self.comm_log_enabled:
            if len(s) < 150:
                print "Stdout received (", len(s), ")", s
            else:
                print "Stdout received (", len(s), ")", s[:70], " ... ", s[-70:]

        if s == "":
            self.add_error("Unexpected EOF of stdout (hda-emu crashed?)", "Fatal")
            return
        self.stdout_total += s

        q, _, self.stdout_total = self.stdout_total.rpartition('\n')
        l = q.splitlines()
        self.stdout_lines += l
        for s in l:
            if s.startswith("Error:"):
                self.add_error(s, "Error")
            if s.startswith("Warning:"):
                self.add_error(s, "Warning")

    def check_stderr(self):
        s = os.read(self.child.stderr.fileno(), 65536)

        if self.comm_log_enabled:
            print "Stderr received (", len(s), ")", s

        if s == "":
            self.add_error("Unexpected EOF of stderr (hda-emu crashed?)", "Fatal")
            return False
        # consume readline echo
        while len(s) > 0 and self.readline_echo.startswith(s[0]):
            self.readline_echo = self.readline_echo[1:]
            s = s[1:]

        # detect prompts and half prompts
        fullprompt = False
        if self.has_partial_stderr_prompt:
             if s.startswith(" "):
                  fullprompt = True
                  s = s[1:]
             else:
                  s = ">" + s
             self.has_partial_stderr_prompt = False
        if s.endswith("> "):
            fullprompt = True
            s = s[:-2]
        elif s.endswith(">"):
            self.has_partial_stderr_prompt = True
            s = s[:-1]

        if s != "":
            self.add_error("Unexpected stderr output: '" + s + "'", "Error")
        return fullprompt


    def run_command(self, command=None):
        self.last_command_printed = False
        self.has_partial_stderr_prompt = False

        if command:
            self.readline_echo = command + '\n'
            self.last_command = command
            if self.comm_log_enabled:
                print "Sending command: ", command
            self.child.stdin.write(command + '\n')
            self.child.stdin.flush()
        else:
            self.readline_echo = ""

        import select
        self.stdout_total = ""
        self.stderr_total = ""
        self.stdout_lines = []
        pipelist = [self.child.stdout, self.child.stderr]
        timeout = 3
        while True:
            readable, _, broken = select.select(pipelist, [], pipelist, timeout)
            for q in broken:
                self.add_error("Broken pipe (hda-emu crashed?)", "Fatal")
                return None
            if self.child.stdout in readable:
                self.check_stdout()
            if self.child.stderr in readable:
                if self.check_stderr():
                    timeout = 0 # One more check round to help against races between stderr and stdout

            if readable == [] and timeout > 0:
                self.add_error("Timeout waiting for hda-emu", "Fatal")
                return None
            elif readable == [] and timeout == 0:
                return self.stdout_lines

    def find_jack_pins(self):
        dump = self.run_command("dump")
        pins = []

        pinregex = re.compile("^Node (0x\w\w+) \\[Pin Complex\\].*")
        jackregex = re.compile("Pin Default.*\\[Jack\\]")
        for s in dump:
            g = pinregex.match(s)
            if g:
                currentpin = g.group(1)
            elif jackregex.search(s):
                pins.append(currentpin)
        return pins

    def run_jack_plug_test(self):
        pins = self.find_jack_pins()
        for pin in pins:
            self.run_command("jack " + pin + " 1")
        for pin in pins:
            self.run_command("jack " + pin + " 0")

    def run_set_kcontrol_test(self, c, values):
        if len(c.values) < len(values):
            values = values[:len(c.values)]
        self.run_command("set " + c.numid + " " + " ".join(values))
        c.add_info(self.run_command("get " + c.numid))
        for a, b in zip(values, c.values):
            if (int(a) != int(b)):
                self.add_error("Tried to set " + c.name + " to " + str([int(x) for x in values]) + ", but got " + str(c.values) + " instead", "Error")

    def run_kcontrol_test(self):
        minmax_regex = re.compile("MIN/MAX: (\d+)/(\d+),\s+VAL:(( \\[\d+\\])+)")
        val_regex = re.compile(" \\[(\d+)\\]")

        controls = self.run_command("list")
        for control in controls:
            c = ControlInfo(self, control)
            c.add_info(self.run_command("get " + c.numid))
            if "minval" in c.__dict__:
                minval = c.minval
                maxval = c.maxval
                # Set to: 1) min, max 2) max, min 3) min, min 4) max, max
                self.run_set_kcontrol_test(c, [minval, maxval])
                self.run_set_kcontrol_test(c, [maxval, minval])
                self.run_set_kcontrol_test(c, [minval, minval])
                self.run_set_kcontrol_test(c, [maxval, maxval])

    def run_pcm_test(self):
        pcm_regex = re.compile("Info: (\d+):.*play=(\d+), capt=(\d+)")
        pcm_lines = self.run_command("PCM")
        playback_test = False
        for pcm_line in pcm_lines:
            r = pcm_regex.match(pcm_line)
            if r is None:
                self.add_error("Invalid pcm response: " + pcm_line, "Error");
                continue
            pcm_devid = r.group(1)
            play_count = r.group(2)
            rec_count = r.group(3)
            if play_count > 0:
                playback_test = True
                self.run_command("PCM " + pcm_devid + " playback")
            if rec_count > 0:
                self.run_command("PCM " + pcm_devid + " capture")
        if not playback_test:
            self.add_error("No playback PCM devices", "Error")

    def run_standard(self):
        self.start_process()
        self.run_command() # Initial parsing
        self.run_command("pm") # S3 test
        self.run_jack_plug_test()
        self.run_kcontrol_test()
        self.run_pcm_test()
        self.stop_process()

