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

#include "textile.h"

#include <assert.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>

/* LCS */
struct lcs_char {
    size_t i;
    size_t j;
    char ch;
};

struct lcs_string {
    struct lcs_char *lcs;
    size_t len;
};

struct c_table_entry {
    unsigned short c;
    unsigned short g;
};

struct c_table {
    struct c_table_entry *table;
    size_t m;
    size_t n;
};

struct c_table_entry
c_get(struct c_table c, size_t i, size_t j)
{
    static struct c_table_entry zero = { .c = 0, .g = 0 };

    if (i>=c.m || j>=c.n)
        return zero;

    return c.table[c.n*i + j];
}

void
c_set(struct c_table c, size_t i, size_t j, struct c_table_entry value)
{
    c.table[c.n*i + j] = value;
}

size_t max(size_t a, size_t b) {
    return (a>b) ? a : b;
}

bool
take_match(struct c_table c,
           const char *x, size_t i, size_t m,
           const char *y, size_t j, size_t n
           ) {
    struct c_table_entry current, down, right, diagonal;

    current = c_get(c, i, j);
    down = c_get(c, i+1, j);
    right = c_get(c, i, j+1);

    /* Take any opportunity to match sooner than later */
    if (current.c > down.c && current.c > right.c) {
        /* Can't find LCS without this match */
        return true;
    } else if (current.g > down.g && current.g > right.g) {
        /* This match is the only way to find the best grouping. */
        return true;
    } else if (x[i]==y[j]) {
        /* We don't need the match for LCS or best grouping */
        /* Still, take the match if we can */
        diagonal = c_get(c, i+1, j+1);
        if (current.g == diagonal.g) {
            /* Won't hurt to take this match */
            return true;
        } else {
            if (current.g == 1 + diagonal.g) {
                /* Take the match only if it is not isolated. */
                if (i && j && x[i-1]==y[j-1]) {
                    /* This match groups with the previous match */
                    return true;
                }
            }
        }
    }
    return false;
}

/* lcs
 *
 * Computes the longest common substring of the two strings passed.
 *
 * x - pointer to the first string (not null-terminated)
 * m - length of the first string
 * y - pointer to the second string (not null-terminated)
 * n - length of the second string
 *
 * result - A string of struct lcs_char that represents the result.
 *          It is assumed that memory has been allocated sufficient to hold
 *          the result which could be as long as the shorter of the two
 *          strings.
 *
 * This algorithm was based on the LCS section of "Introduction to Algoritms"
 * by Thomas Cormen, Charles Leiserson, Ronald Rivest and Clifford Stein.
 *
 * A few improvements have been made:
 *
 * 1) I run the algorithm backwards compare to the book.  This tends to find
 *    matches earlier in the string.
 * 2) I added the "g" table which allows me to find an LCS with the most
 *    "grouping" possible.  This is because merging is more difficult when
 *    the common parts are found fragmented all throughout the original strings.
 *    Conflicts are also much more difficult to understand.
 *    Note that this optimization doubles the already greedy memory requirements
 *    of this algorithm.  It most likely adds a constant factor to the runtime
 *    as well.
 */
static size_t
lcs(const char *x, size_t m, const char *y, size_t n, struct lcs_char *result)
{
    struct lcs_char *entry;
    struct c_table_entry c_entry, current, down, right, diagonal;
    size_t i, j, length;

    struct c_table c  = { .m = m, .n = n, .table = 0 };

    if (! (x && m && y && n && result)) {
        return 0;
    }

    if (USHRT_MAX < m || USHRT_MAX < n) {
        return 0;
    }

    /* This can be quite large. */
    c.table = malloc(m * n * sizeof (struct c_table_entry));

    if (!c.table) {
        return 0;
    }

    /* Based on pseudo-code from LCS-LENGTH(X,Y) */
    /* Here's the O(mn).  Computes the c table from the book. */
    for (i=m; i!=0; --i) {
        for (j=n; j!=0; --j) {
            down = c_get(c, i, j-1);
            right = c_get(c, i-1, j);
            diagonal = c_get(c, i, j);

            c_entry.c = (x[i-1]==y[j-1]) ? diagonal.c+1 : max(down.c, right.c);

            c_entry.g = 0;
            if (down.c == c_entry.c)
                c_entry.g = down.g;

            if (right.c == c_entry.c)
                c_entry.g = max(right.g, c_entry.g);

            if (x[i-1]==y[j-1]) {
                c_entry.g = max(diagonal.g, c_entry.g);
                if (i!=m && j!=n && x[i]==y[j]) {
                    c_entry.g = max(diagonal.g+1, c_entry.g);
                }
            }

            c_set(c, i-1, j-1, c_entry);
        }
    }

    /* Adapted from PRINT-LCS(X,i,j) */
    length = c_get(c, 0, 0).c;
    for (i = 0, j = 0; i != m && j != n; ) {
        current = c_get(c, i, j);
        if (!current.c)
            break;

        if (take_match(c, x, i, m, y, j, n)) {
            entry = &result[length - current.c];
            entry->ch = x[i];
            entry->i = i++;
            entry->j = j++;
        } else {
            down = c_get(c, i+1, j);
            right = c_get(c, i, j+1);

            if (down.c != right.c ? down.c > right.c : down.g > right.g)
                i++;
            else
                j++;
        }
    }

    free(c.table);

    return length;
}

struct cursor {
    size_t index;

    size_t i_len;
    size_t i_begin;
    size_t i_end;

    size_t j_len;
    size_t j_begin;
    size_t j_end;

    struct lcs_string *str;
};

void cursor_init(struct cursor *c, struct lcs_string *str, size_t i_len, size_t j_len) {
    c->index = 0;
    c->i_begin = 0;
    c->j_begin = 0;
    c->i_len = i_len;
    c->j_len = j_len;
    c->str = str;

    if (c->index == c->str->len) {
        c->i_end = c->i_len;
        c->j_end = c->j_len;
    } else {
        c->i_end = c->str->lcs[c->index].i;
        c->j_end = c->str->lcs[c->index].j;
    }
}

void cursor_advance(struct cursor *c, bool advance_begin) {
    c->index++;

    if (advance_begin) {
        c->i_begin = c->i_end;
        c->j_begin = c->j_end;
    }

    if (c->index >= c->str->len) {
        c->i_end = c->i_len;
        c->j_end = c->j_len;
    } else {
        c->i_end = c->str->lcs[c->index].i;
        c->j_end = c->str->lcs[c->index].j;
    }
}

/* Notes:
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
        void *data)
{
    struct cursor src, dest;
    struct lcs_string src_lcs, dest_lcs;
    bool conflicts_found = false, conflict, take_theirs;
    bool equal, only_deletes;
    size_t match_length, old_end;

    /* Compute LCS between base and theirs. */
    src_lcs.len = max(base_len, theirs_len);
    src_lcs.lcs = malloc (src_lcs.len * sizeof (struct lcs_char));
    if (src_lcs.lcs) {
        src_lcs.len = lcs(base, base_len, theirs, theirs_len, src_lcs.lcs);
    } else {
        src_lcs.len = 0;
    }
    cursor_init(&src, &src_lcs, base_len, theirs_len);

    /* Compute LCS between base and ours. */
    dest_lcs.len = max(base_len, ours_len);
    dest_lcs.lcs = malloc (dest_lcs.len * sizeof (struct lcs_char));
    if (dest_lcs.lcs) {
        dest_lcs.len = lcs(base, base_len, ours, ours_len, dest_lcs.lcs);
    } else {
        dest_lcs.len = 0;
    }
    cursor_init(&dest, &dest_lcs, base_len, ours_len);

    for ( ; src.index <= src_lcs.len && dest.index <= dest_lcs.len;
            cursor_advance(&src, true), cursor_advance(&dest, true) ) {
        assert(src.i_begin == dest.i_begin);

        /**
         * Each time through this loop sets begin to point to a "character"
         * that matches in all three files.  The first time through the loop is
         * special in that the matching "character" is the beginning of the
         * sequence (like ^ in a regular expression.)  It has zero length.
         */

        match_length = src.index ? 1 : 0;

        only_deletes = true;
        if (match_length != (src.j_end - src.j_begin))
            only_deletes = false;
        if (match_length != (dest.j_end - dest.j_begin))
            only_deletes = false;

        /*
         * Find an end that matches in all three files.  Do this by advancing
         * whichever cursor trails in the base file until both cursors point to
         * the same position in the base file.
         *
         * Always guaranteed to find a matching end since EOF will match.
         * Note that this always finds the first such position relative to
         * where the begins were set above.
         */
        while(src.i_end != dest.i_end) {
            if(src.i_end < dest.i_end) {
                old_end = src.j_end;
                cursor_advance(&src, false);
                if (1 != (src.j_end - old_end))
                    only_deletes = false;
            } else {
                old_end = dest.j_end;
                cursor_advance(&dest, false);
                if (1 != (dest.j_end - old_end))
                    only_deletes = false;
            }
        }

        /*
         * i_begin and i_end in each cursor bracket an area where changes have
         * been made in ours, theirs or both.  It is tight in the sense that
         * there are no characters within the bounds that match in all three.
         * Hence, it is not possible to find a smaller subset of changes that
         * are bound by a character common to all three.
         */

        assert(src.i_end == dest.i_end);

        /*
         * Optimize cases where all of the current group of changes are
         * deletes in either ours, theirs or both.
         */
        if (only_deletes) {
            if (match_length) {
                merged(data, ours + dest.j_begin, 1);
            }
            continue;
        }

        conflict = true;
        take_theirs = false;

        /*
         * Three cases here are considered below.
         *
         * 1. Only changed in ours.
         * 2. Only changed in theirs.
         * 3. Changed identically in ours and theirs.
         *
         * Everything else is a conflict.
         */
        if (src.i_end - src.i_begin == src.j_end - src.j_begin) {
            equal = ! strncmp(
                    base + src.i_begin,
                    theirs + src.j_begin,
                    src.i_end - src.i_begin
                    );
            if (equal) {
                /* theirs is the same as base.  Take ours. */
                conflict = false;
            }
        }

        if (conflict && dest.i_end - dest.i_begin == dest.j_end - dest.j_begin) {
            equal = ! strncmp(
                    base + dest.i_begin,
                    ours + dest.j_begin,
                    dest.i_end - dest.i_begin
                    );
            if (equal) {
                /* ours is the same as base.  Take theirs. */
                take_theirs = true;
                conflict = false;
            }
        }

        if (conflict && src.j_end - src.j_begin == dest.j_end - dest.j_begin) {
            equal = ! strncmp(
                    theirs + src.j_begin,
                    ours + dest.j_begin,
                    dest.j_end - dest.j_begin
                    );
            if (equal) {
                /* ours is the same as theirs.  Take ours. */
                conflict = false;
            }
        }

        /*
         * We've identified what content should be used.  Defer to the caller's
         * callback functions to handle it.
         */
        if (!conflict) {
            if (take_theirs) {
                merged(data, theirs + src.j_begin, src.j_end - src.j_begin);
            } else {
                merged(data, ours + dest.j_begin, dest.j_end - dest.j_begin);
            }
        } else {
            conflicts_found = true;

            if (match_length) {
                /*
                 * When index == 0 the "matching character" is the start of the
                 * sequence.  (Like ^ in a regex)  It shouldn't be printed.
                 */
                merged(data, ours + dest.j_begin, match_length);
                dest.j_begin++;
                dest.i_begin++;
                src.j_begin++;
            }

            conflicted( data,
                base + dest.i_begin, dest.i_end - dest.i_begin,
                ours + dest.j_begin, dest.j_end - dest.j_begin,
                theirs + src.j_begin, src.j_end - src.j_begin
            );
        }
    }

    if (dest_lcs.lcs) free(dest_lcs.lcs);
    if (src_lcs.lcs) free(src_lcs.lcs);

    return conflicts_found;
}
