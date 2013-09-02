Experimental Branch
=======

This is my experimental branch.  In the interest of openness, I wanted
the community to be able to see what I'm working on before it is ready
for the master branch.

You should treat the experimental branch similarly to how you'd treat
direct access to my personal repository.  I reserve the right to amend
any of the commits in this branch or even to rebase the entire branch
itself.  It is extremely unlikely that any of the commits in this
branch will make it to the master branch with exactly the same commit
id it has here.

textile
=======

Textile Merge Library

The textile merge library supplements other merge software such as
diff3.  It is capable of performing 3-way merges in many cases where
changes are made on both sides within the same line.

Textile uses an algorithm to find the longest common subsequence (LCS)
between the base and each of the derived files.  It then uses the two
to perform the three-way merge.  When the merge is successful, it
calls a callback provided by the caller.  The caller provides a
similar callback to handle conflicting changes.

The LCS algorithm is very CPU and memory intensive.  It is O(mn) for
both time and space where m and n are the lengths of the sequences.
Some extra effort is made to find the LCS with the least amount of
fragmention.  In other words, the matching parts of the LCS are
consecutive as much as possible.  This doesn't change the algorithm
complexity but does double the amount of memory used and increases the
runtime similarly.

Because of the demands on time and memory, this library is only useful
for resolving smaller conflicts.  It is expected that the caller will
do the heavy lifting by doing most of the merge using traditional line
based merge techniques and use this library as a second pass over the
conflicts.

This library is experimental for now.  I imagine that it will take
some time to get experience with it.  For now, I have created a patch
to diffutils to run the algorithm when --inline-merge is passed.  I
call it from a custom git mergetool script to post process merge
conflicts.  For now, I wish to keep an eye on it.
