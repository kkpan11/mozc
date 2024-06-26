# -*- coding: utf-8 -*-
# Copyright 2010-2021, Google Inc.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#
#     * Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above
# copyright notice, this list of conditions and the following disclaimer
# in the documentation and/or other materials provided with the
# distribution.
#     * Neither the name of Google Inc. nor the names of its
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

"""Utilitis for copying dependent files for Windows build."""

import argparse
import datetime
import os
import shutil


def ParseOption():
  """Parse command line options."""
  parser = argparse.ArgumentParser()
  additional_desc = ' you can use %s as path separator' % os.pathsep
  parser.add_argument(
      '--dll_paths',
      required=True,
      help='Search paths for DLLs.' + additional_desc,
  )
  parser.add_argument(
      '--pdb_paths',
      required=True,
      help='Search paths for PDB files.' + additional_desc,
  )
  parser.add_argument(
      '--target_dir', required=True, help='Deploy target directory.'
  )
  parser.add_argument(
      '--basenames',
      required=True,
      help='The basenames of DLL and/or PDB.' + additional_desc,
  )

  return parser.parse_args()


def DeployMain(full_filename, src_paths, target_absdir):
  """Main deployment logic.

  Args:
    full_filename: A string which represents the filename to be copied.
    src_paths: A string which represents file search path with delimited by a
      OS-dependent path separator.
    target_absdir: A string which represents target directory name.
  """
  if not src_paths:
    return

  target_file_abspath = os.path.join(target_absdir, full_filename)
  target_abs_dir = os.path.dirname(target_file_abspath)
  if not os.path.exists(target_abs_dir):
    os.makedirs(target_abs_dir)

  def _GetLastModifiedTime(filepath):
    """Returns the last modified time of a file.

    Args:
      filepath: A string which represents the filename to be checked.

    Returns:
      A Datetime object which represents the last modified time of the
      specified filename. If the file does not exist, returns epoch time +1 day.
    """
    if not os.path.isfile(filepath):
      # The epoch time doesn't work in some timezones. 86400 = 24 * 60 * 60.
      return datetime.datetime.fromtimestamp(86400)
    return datetime.datetime.fromtimestamp(os.path.getmtime(filepath))

  target_file_mtime = _GetLastModifiedTime(target_file_abspath)

  for path in src_paths.split(os.pathsep):
    src = os.path.abspath(os.path.join(path, full_filename))
    if not os.path.isfile(src):
      continue
    if _GetLastModifiedTime(src) <= target_file_mtime:
      # Older file found. Ignore.
      continue
    print('Copying %s to %s' % (src, target_file_abspath))
    shutil.copy2(src, target_file_abspath)
    break


def main():
  opts = ParseOption()

  target_absdir = os.path.abspath(opts.target_dir)
  for basename in opts.basenames.split(os.pathsep):
    DeployMain(basename + '.dll', opts.dll_paths, target_absdir)
    DeployMain(basename + '.pdb', opts.pdb_paths, target_absdir)


if __name__ == '__main__':
  main()
