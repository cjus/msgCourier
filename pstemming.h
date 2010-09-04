/* This is the Porter stemming algorithm, coded up in ANSI C by the
   author. It may be be regarded as cononical, in that it follows the
   algorithm presented in

   Porter, 1980, An algorithm for suffix stripping, Program, Vol. 14,
   no. 3, pp 130-137,

   only differing from it at the points maked --DEPARTURE-- below.

   See also http://www.tartarus.org/~martin/PorterStemmer

   The algorithm as described in the paper could be exactly replicated
   by adjusting the points of DEPARTURE, but this is barely necessary,
   because (a) the points of DEPARTURE are definitely improvements, and
   (b) no encoding of the Porter stemmer I have seen is anything like
   as exact as this version, even with the points of DEPARTURE!

   You can compile it on Unix with 'gcc -O3 -o stem stem.c' after which
   'stem' takes a list of inputs and sends the stemmed equivalent to
   stdout.

   The algorithm as encoded here is particularly fast.

   Release 1
*/

#ifndef PSTEMMING_H
#define PSTEMMING_H

#include <string.h>
#include "buffer.h"

class cPorterStemming
{
public:
	cPorterStemming() {};
	~cPorterStemming() {};

	int Process(char *pszString);

private:
	cBuffer m_Word;
	cBuffer m_InBuffer;
	cBuffer m_OutBuffer;

	/* The main part of the stemming algorithm starts here. b is a buffer
	   holding a word to be stemmed. The letters are in b[k0], b[k0+1] ...
	   ending at b[k]. In fact k0 = 0 in this demo program. k is readjusted
       downwards as the stemming progresses. Zero termination is not in fact
       used in the algorithm.

       Note that only lower case sequences are stemmed. Forcing to lower case
       should be done before stem(...) is called.
     */

	char * b;       /* buffer for word to be stemmed */
	int k,k0,j;     /* j is a general offset into the string */

	int cons(int i);
	int cPorterStemming::m();
	int vowelinstem();
	int doublec(int j);
	int cvc(int i);
	int ends(char * s);
	void setto(char * s);
	void r(char * s);
	void step1ab();
	void step1c();
	void step2();
	void step3();
	void step4();
	void step5();
	int stem(char * p, int i, int j);

};

#endif //PSTEMMING_H
