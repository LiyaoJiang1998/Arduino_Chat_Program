/*
Name: LIYAO JIANG(1512446)
Section#: EA1

Name：XIAOLEI ZHANG（1515335）
Section#：LBL A1
*/
#include <Arduino.h> // for uint32_t and friends
#include "powmod.h"

//when doing the multiplication of 31-bits numbers
//we might facing overflow using uint32_t type
//by using mulMod to avoid overflow even using uint32_t
//it is returns the result of (a*b) mod m
uint32_t mulMod(uint32_t a, uint32_t b, uint32_t m) {
	//take a, b mod by m at the start to become 31-bit
	a= a%m;
	b= b%m;
	uint32_t bi= (1*b) % m;//the variable to store (2^i)*b mod m every iteration
	uint32_t result= 0;//the variable to store the result
	for (int i=0; i<31; i++){
		//read the 31bit number off as binary sum digit by digit
		int ai = (a >> i) & 1;
		//mod m for each multiplication and adding to avoid overflow
		result = (result + (ai*bi)) % m;
		bi = (2 * bi) % m;
	}
	return result;
}


/*
	Compute and return (a to the power of b) mod m.
 	Assumes 1 <= m < 2^16 (i.e. that m fits in a uint16_t).
	Example: powMod(2, 5, 13) should return 6.
*/
uint32_t powModFast(uint32_t a, uint32_t b, uint32_t m) {
	// compute ap[0] = a
	// ap[1] = ap[0]*ap[0]
	// ...
	// ap[i] = ap[i-1]*ap[i-1] (all mod m) for i >= 1

	uint32_t result = 1%m;
	uint32_t sqrVal = a%m; //stores a^{2^i} values, initially 2^{2^0}
	uint32_t newB = b;

	// LOOP INVARIANT: statements that hold throughout a loop
	//                 with the goal of being convinced the loop works
	// statements: just before iteration i,
	//               result = a^{binary number represented the first i bits of b}
	//               sqrVal = a^{2^i}
	//               newB = b >> i
	while (newB > 0) {
		if (newB&1) { // evalutates to true iff i'th bit of b is 1
			result = mulMod(result,sqrVal,m);
		}
		sqrVal = mulMod(sqrVal,sqrVal,m);
		newB = (newB>>1);
	}

	// upon termination: newB == 0
	// so b >> i is 0
	// so result a^{binary number represented the first i bits of b} = a^b, DONE!

	// # iterations ~ log_2 b ~ # of bits to write b
	// running time = O(log b)
	// NOTATION: we write O(some function) to express how the running times
	//           grows as a function of the input

	return result;
}




// /*
// 	Check if powMod(a, b, m) == result
// */
// void onePowModFastTest(uint32_t a, uint32_t b, uint32_t m,
// 									 uint32_t expected) {
// 	uint32_t calculated = powModFast(a, b, m);
// 	if (calculated != expected) {
// 		Serial.println("error in powMod test");
// 		Serial.print("expected: ");
// 		Serial.println(expected);
// 		Serial.print("got: ");
// 		Serial.println(calculated);
// 	}
// }
//
// void testPowModFast() {
// 	Serial.println("starting tests");
// 	// run powMod through a series of tests
// 	onePowModFastTest(1, 1, 20, 1); //1^1 mod 20 == 1
// 	onePowModFastTest(5, 7, 37, 18);
// 	onePowModFastTest(5, 27, 37, 31);
// 	onePowModFastTest(3, 0, 18, 1);
// 	onePowModFastTest(2, 5, 13, 6);
// 	onePowModFastTest(1, 0, 1, 0);
// 	onePowModFastTest(123456, 123, 17, 8);
//
// 	Serial.println("Now starting big test");
// 	uint32_t start = micros();
// 	onePowModFastTest(123, 2000000010ul, 17, 16);
// 	uint32_t end = micros();
// 	Serial.print("Microseconds for big test: ");
// 	Serial.println(end-start);
//
// 	Serial.println("Another big test");
// 	onePowModFastTest(123456789, 123456789, 61234, 51817);
//
// 	Serial.println("With a big modulus (< 2^31)");
// 	onePowModFastTest(123456789, 123456789, 2147483647, 1720786551);
//
//
// 	Serial.println("tests done!");
// }
