/**
 * © Copyright 2013 Carl N. Baldwin
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _LCS_MERGE_H
#define _LCS_MERGE_H

#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Merges changes made between base and theirs in to ours.  It is assumed that
 * base was the common starting point for ours and theirs.
 *
 * While most merge tools work at the line level this function works at the
 * byte level.  It is capable of merging where both files have changes to the
 * same line.
 *
 * Conflict markers are similar to diff3.  However, the conflict markers do not
 * appear on a line by themselves.  Since this is a byte-level merge the
 * conflict markers appear inline.
 *
 * The following sequences are not null terminated.  The length of each
 * sequence must be passed using the _len arguments.
 *
 * base - Pointer to the sequence that serves as the common base.
 * ours - Pointer to our sequence.
 * theirs - Pointer to their sequence.
 *
 * The following arguments are callback funtions called to handled merged or
 * conflicted sections.
 *
 * merged - Receives a single merged sequence.  Can be copied to output.
 * conflicted - Receives three sequences:  base, ours, theirs in that order.
 *              A section that could not be resolved.
 *
 * handlerData - A void pointer that will be passed to each of the callbacks.
 *
 * Returns true if conflicts occurred.
 */
bool
textile_merge(
        const char *base, size_t base_len,
        const char *ours, size_t ours_len,
        const char *theirs, size_t theirs_len,
        void (*merged)(void *, const char *, size_t),
        void (*conflicted)(void *,
            const char *, size_t,
            const char *, size_t,
            const char *, size_t),
        void *handlerData);

#ifdef __cplusplus
}
#endif

#endif
