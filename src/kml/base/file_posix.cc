// Copyright 2008, Google Inc. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without 
// modification, are permitted provided that the following conditions are met:
//
//  1. Redistributions of source code must retain the above copyright notice, 
//     this list of conditions and the following disclaimer.
//  2. Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//  3. Neither the name of Google Inc. nor the names of its contributors may be
//     used to endorse or promote products derived from this software without
//     specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
// EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
// OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// The file contains the implementation of the File methods specific to
// POSIX platforms.

#include "kml/base/file.h"
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#ifdef _WIN32
/* mkstemp extracted from libc/sysdeps/posix/tempname.c.  Copyright
 (C) 1991-1999, 2000, 2001, 2006 Free Software Foundation, Inc.

 The GNU C Library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.  */

static const char letters[] =
"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

/* Generate a temporary file name based on TMPL.  TMPL must match the
 rules for mk[s]temp (i.e. end in "XXXXXX").  The name constructed
 does not exist at the time of the call to mkstemp.  TMPL is
 overwritten with the result.  */
int
mkstemp (char *tmpl)
{
  int len;
  char *XXXXXX;
  static unsigned long long value;
  unsigned long long random_time_bits;
  unsigned int count;
  int fd = -1;
  int save_errno = errno;

  /* A lower bound on the number of temporary files to attempt to
     generate.  The maximum total number of temporary file names that
     can exist for a given template is 62**6.  It should never be
     necessary to try all these combinations.  Instead if a reasonable
     number of names is tried (we define reasonable as 62**3) fail to
     give the system administrator the chance to remove the problems.  */
#define ATTEMPTS_MIN (62 * 62 * 62)

  /* The number of times to attempt to generate a temporary file.  To
     conform to POSIX, this must be no smaller than TMP_MAX.  */
#if ATTEMPTS_MIN < TMP_MAX
  unsigned int attempts = TMP_MAX;
#else
  unsigned int attempts = ATTEMPTS_MIN;
#endif

  len = strlen (tmpl);
  if (len < 6 || strcmp (&tmpl[len - 6], "XXXXXX"))
    {
    errno = EINVAL;
    return -1;
    }

  /* This is where the Xs start.  */
  XXXXXX = &tmpl[len - 6];

  /* Get some more or less random data.  */
  {
    SYSTEMTIME      stNow;
    FILETIME ftNow;

    // get system time
    GetSystemTime(&stNow);
    stNow.wMilliseconds = 500;
    if (!SystemTimeToFileTime(&stNow, &ftNow))
    {
      errno = -1;
      return -1;
    }

    random_time_bits = (((unsigned long long)ftNow.dwHighDateTime << 32)
              | (unsigned long long)ftNow.dwLowDateTime);
  }
  value += random_time_bits ^ (unsigned long long)GetCurrentThreadId ();

  for (count = 0; count < attempts; value += 7777, ++count)
    {
    unsigned long long v = value;

    /* Fill in the random bits.  */
    XXXXXX[0] = letters[v % 62];
    v /= 62;
    XXXXXX[1] = letters[v % 62];
    v /= 62;
    XXXXXX[2] = letters[v % 62];
    v /= 62;
    XXXXXX[3] = letters[v % 62];
    v /= 62;
    XXXXXX[4] = letters[v % 62];
    v /= 62;
    XXXXXX[5] = letters[v % 62];

    fd = open (tmpl, O_RDWR | O_CREAT | O_EXCL, _S_IREAD | _S_IWRITE);
    if (fd >= 0)
    {
      errno = save_errno;
      return fd;
    }
    else if (errno != EEXIST)
      return -1;
    }

  /* We got out of the loop because we ran out of combinations to try.  */
  errno = EEXIST;
  return -1;
}
#endif


namespace kmlbase {

// Internal to the POSIX File class.
static bool StatFile(const char* path, struct stat* stat_data) {
  struct stat tmp;
  if (stat(path, &tmp) !=0) {
    return false;
  }
  *stat_data = tmp;
  return true;
}

bool File::Exists(const string& full_path) {
  struct stat stat_data;
  if (!StatFile(full_path.c_str(), &stat_data)) {
    return false;
  }
  return S_ISREG(stat_data.st_mode);
}

bool File::Delete(const string& filepath) {
  return unlink(filepath.c_str()) == 0;
}

bool File::CreateNewTempFile(string* path) {
  if (!path) {
    return false;
  }
  char temp_path[] = "/tmp/libkmlXXXXXX";
  int fd = mkstemp(temp_path);
  if (fd == -1) {
    return false;
  }
  close(fd);
  path->assign(temp_path, strlen(temp_path));
  return true;
}

}  // end namespace kmlbase
