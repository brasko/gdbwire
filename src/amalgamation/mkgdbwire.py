#!/usr/bin/python3
#
# This script creates the amalgamation files, gdbwire.h and gdbwire.c.
#
# The build system passes this script the gdbwire version, the
# root to the source directory and the root to the build directory.
# This script finds the appropriate files and copies them into 
# the temporary build directory, src/amalgamation/tsrc. The
# easiest way to create this directory is to configure with
# --enable-amalgamation enabled and to do a build of gdbwire with
# the 'make' build command. Note that some of the copied header
# and source files are generated from flex and bison.
#
# After the "tsrc" directory has been created and populated, the build
# system runs the script:
#
#      python3 mkgdbwire.py package_version abs_source_path abs_build_path
#
# The amalgamated gdbwire code will be written into gdbwire.h and gdbwire.c.

import glob
import os.path
import re
import shutil
import subprocess
import sys

assert(len(sys.argv) == 4)
package_version = sys.argv[1]
abs_source_path = sys.argv[2]
abs_build_path = sys.argv[3]

# Note, it may have been possible to glob the below header_files and
# source_files from the tsrc directory. However, for now, I'd like to
# list them out for a few reasons. The first is that it provides
# consistancy in the amalgamation output files. The files will always
# be processed in the same order. The second is, it provides a safety
# check to always ensure that known files are included in the amalgamation.
#
# This means if an include or source file is added to automake, it will
# also have to be updated here as well.

# These are the header files used by gdbwire.
header_files = [
    'gdbwire_sys.h',
    'gdbwire_string.h',
    'gdbwire_assert.h',
    'gdbwire_result.h',
    'gdbwire_logger.h',
    'gdbwire_mi_pt.h',
    'gdbwire_mi_pt_alloc.h',
    'gdbwire_mi_parser.h',
    'gdbwire_mi_command.h',
    'gdbwire_mi_grammar.h',
    'gdbwire.h']

# These are the soruce files used by gdbwire
source_files = [
    'gdbwire_sys.c',

    'gdbwire_string.c',

    'gdbwire_logger.c',
    'gdbwire_mi_parser.c',
    'gdbwire_mi_pt_alloc.c',
    'gdbwire_mi_pt.c',
    'gdbwire_mi_command.c',

    'gdbwire_mi_lexer.c',
    'gdbwire_mi_grammar.c',

    'gdbwire.c',
]

def comment(out, text):
    end_stars = '*' * (70 - len(text))
    out.write('/***** ' + text + ' ' + end_stars + '/\n')

# include and line directive regular expressions
include_regex = re.compile('\s*#\s*include\s+"([^"]+)"')
line_regex = re.compile("#line")

# Copy the file filename into the output.
# If any #include statements are seen, either
# - inline the file (if it's the first time the file has been seen)
# - comment out the #include (if it's not the first time)
# - or leave it alone if it's not a gdbwire include
#
# The first time a header file from available_hdr is seen
# in a #include statement in the code, include the complete
# text of the file in-line.
#
# @param out
# The output file to write to
#
# @param available_hdr
# A map of filename to boolean determining if a particular header file
# still needs to be expanded.
#
# @param filename
# The filename currently being copied into the output file
def copy_file(out, available_hdr, filename):
    name = os.path.basename(filename)
    comment(out, "Begin file " + name)
    with open(filename, 'r') as fd:
        for line in fd:
            m = include_regex.match(line)
            if m:
                hdr = m.group(1)
                if hdr in header_files and available_hdr[hdr]:
                    available_hdr[hdr] = False
                    comment(out, "Include " + hdr + " in the middle of " + name)
                    copy_file(out, available_hdr, "tsrc/" + hdr)
                    comment(out, "Continuing where we left off in " + name)
                else:
                  # Comment out the entire line, replacing any nested comment
                  # begin/end markers with the harmless substring "**".
                  out.write("/* %s */\n" % line.rstrip())
            elif line_regex.match(line):
                # Skip #line directives.
                None
            else:
                out.write(line)
    comment(out, "End of " + name)

def write_copyright(fd, package_version, rev):
    fd.write(
"""/**
 * Copyright (C) 2013 Robert Rossi <bob@brasko.net>
 *
 * This file is an amalgamation of the source files from GDBWIRE.
 *
 * It was created using gdbwire %(ver)s and git revision %(rev)s.
 *
 * GDBWIRE is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * GDBWIRE is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GDBWIRE.  If not, see <http://www.gnu.org/licenses/>.
 */

""" % { 'ver': package_version, 'rev': rev })

def write_gdbwire_source(filename, package_version, rev):
    available_hdr = {}
    for hdr in header_files:
        available_hdr[hdr] = True

    out = open(filename, "w")

    write_copyright(out, package_version, rev)

    for file in source_files:
      copy_file(out, available_hdr, 'tsrc/' + file)

def write_gdbwire_header(filename, package_version, rev):
    available_hdr = {}
    for hdr in header_files:
        available_hdr[hdr] = True

    out = open(filename, "w")
    
    write_copyright(out, package_version, rev)

    for file in header_files:
      copy_file(out, available_hdr, 'tsrc/' + file)

# Create the "tsrc" directory
def create_tsrc():
    if not os.path.exists("tsrc"):
        os.mkdir("tsrc")
    for filename in header_files + source_files:
        source_path = os.path.join(abs_source_path, 'src', filename)
        build_path = os.path.join(abs_build_path, 'src', filename)
        if os.path.exists(source_path):
            shutil.copyfile(source_path, os.path.join("tsrc", filename))
        elif os.path.exists(build_path):
            shutil.copyfile(build_path, os.path.join("tsrc", filename))
        else:
            raise Exception("Path " + filename + " could not be found.")

def get_git_revision():
    # Get the git revision used to make this amalgamation
    command = ["git", "rev-parse", '--short', 'HEAD']
    p = subprocess.Popen(command, stdout=subprocess.PIPE, cwd=abs_source_path)
    rev = p.communicate()[0].strip()
    if p.returncode != 0:
        rev = "Unknown"
    return rev

create_tsrc()
rev = get_git_revision()
write_gdbwire_header('gdbwire.h', package_version, rev)
write_gdbwire_source('gdbwire.c', package_version, rev)
