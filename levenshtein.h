///
/// Levenshtein distance. 
///
/// The smallest number of insertions, deletions, and substitutions required
/// to change one string or tree into another. (2) A T(m × n) algorithm to 
/// compute the distance between strings, where m and n are the lengths of 
/// the strings. 
///
///	http://www.nist.gov/dads/HTML/Levenshtein.html
///
/// Additional comments prefixed with CJ: by Carlos Justiniano
///
///
/// One historically significant multiple-relation similarity test is the 
/// Levenshtein Metric [2] which calculates the difference between strings 
/// in terms of an edit distance, or the minimum number of basic editing 
/// operations which can transform one string into the other. Typical basic 
/// editing operations are insertion, deletion, substitution and adjacent 
/// transposition. These particular operations are especially important in 
/// text processing applications [3] because they represent the most common 
/// forms of typing errors [4]. Such edit distance algorithms classify strings 
/// as similar when their edit distance is less than some threshold, k. 
/// Not surprisingly, this problem is commonly called the k differences problem 
/// in the literature. This problem is also known as the evolutionary distance 
/// problem in the context of molecular biology [5] and the string-to-string 
/// correction problem in text processing circles [6]. There are also several 
/// variations on this general theme, including the k mismatches problem which 
/// locates all occurrences of a string in a corpus which have no more than k 
/// mismatches. For excellent surveys of the underlying ASM algorithms, see 
/// Galil and Giancarlo [7], and Ukkonen [8].
/// http://www.acm.org/~hlb/publications/asm/asm.html
///

#ifndef _LEVENSHTEIN_H
#define _LEVENSHTEIN_H

#include <string>
#include "levenshtein.h"

class cLevenshtein
{
public:
	static int distance(const std::string &source, std::string &target);
};

#endif //_LEVENSHTEIN_H
