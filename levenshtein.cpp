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

#include <vector>
#include "levenshtein.h"
#include "log.h"
#include "exception.h"

using namespace std;

int cLevenshtein::distance(const string &source, string &target) 
{
	int iRet = 0;	
	try
	{	
		// http://www.merriampark.com/ld.htm
		// this version adopted from: http://www.merriampark.com/ldcpp.htm

		// Step 1
		// CJ: If length of source is zero then the number of operations is equal to the
		//     length of the target string or if the length of the target string is zero
		//     then the total operations is equal to the length of the source string.
		string::size_type n = source.length();
		string::size_type m = target.length();
		if (n == 0) 
			return m;
		if (m == 0) 
			return n;

		// Good form to declare a TYPEDEF
		typedef std::vector< std::vector<int> > Tmatrix; 
		Tmatrix matrix(n+1);

		// Size the vectors in the 2.nd dimension. Unfortunately C++ doesn't
		// allow for allocation on declaration of 2.nd dimension of vec of vec
		// CJ: Construct a matrix containing 0..m rows and 0..n columns. 
		for (string::size_type i = 0; i <= n; i++) 
	    	matrix[i].resize(m+1);

		// Step 2
		// 
		for (string::size_type i = 0; i <= n; i++)
			matrix[i][0] = i;
		for (string::size_type j = 0; j <= m; j++)
			matrix[0][j] = j;

		// Step 3
		for (string::size_type i = 1; i <= n; i++) 
		{
		    const char s_i = source[i-1];

			// Step 4
		    for (string::size_type j = 1; j <= m; j++) 
			{
				char t_j = target[j-1];

				// Step 5
				int cost;
				if (s_i == t_j) 
					cost = 0;
				else 
					cost = 1;

				// Step 6
				int above = matrix[i-1][j];
				int left = matrix[i][j-1];
				int diag = matrix[i-1][j-1];
				int cell = min(above + 1, min(left + 1, diag + cost));

				// Step 6A: Cover transposition, in addition to deletion,
				// insertion and substitution. This step is taken from:
				// Berghel, Hal ; Roach, David : "An Extension of Ukkonen's 
				// Enhanced Dynamic Programming ASM Algorithm"
				// (http://www.acm.org/~hlb/publications/asm/asm.html)
				if (i>2 && j>2) 
				{
					int trans = matrix[i-2][j-2]+1;
					if (source[i-2] != t_j) 
						trans++;
					if (s_i != target[j-2]) 
						trans++;
					if (cell>trans) 
						cell=trans;
				}

				matrix[i][j] = cell;
			}
		}

		// Step 7
		// CJ: Return the distance which is found in the lower right hand 
		//     corner of the matrix
		iRet = matrix[n][m];
	}
	catch (exception const &e)
	{
		LOGALL(e.what());	
	}
	return iRet;
}

