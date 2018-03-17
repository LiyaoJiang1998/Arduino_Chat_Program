/*
Name: LIYAO JIANG(1512446)
Section#: EA1

Name：XIAOLEI ZHANG（1515335）
Section#：LBL A1

This chatting program achieved arduino-to-arduino communication over an
encrypted serial link in which the Diffie-Hellman shared secret protocol is used
to establish a shared key. And it uses XOR encryption to encrypt the messages.
Users can simply type to communicate safetly.
*/


#include <Arduino.h>
#include "fsm_client_server.h"
#include "powmod.h"

//the floating analog pin 1 used to create random number
int randomNumPin= A1;
//the pin used for identifacation of server or client for handshake
int idPin=13;

void setup() {
	init();
	// initialize the serial monitor and setup the input pin
	Serial.begin(9600);
	Serial3.begin(9600);
	pinMode(randomNumPin,INPUT);
	pinMode(idPin,INPUT);
}

//rolling cipher function copied from description
uint32_t next_key(uint32_t current_key) {
  const uint32_t modulus = 0x7FFFFFFF; // 2^31-1
  const uint32_t consta = 48271;  // we use that consta<2^16
  uint32_t lo = consta*(current_key & 0xFFFF);
  uint32_t hi = consta*(current_key >> 16);
  lo += (hi & 0x7FFF)<<16;
  lo += hi>>15;
  if (lo > modulus) lo -= modulus;
  return lo;
}

//use this function to generate a random 32-bits number
uint32_t randomNum(){
	//each time take the least significant digit of the 10-bit random number
	//take 32 times to get a 32-bit random number

	//initialize a string to save the random number generated
	char str[32];// 32bits+ 1 null terminator
	for(int i=0; i<32; i++){
		//use analogRead of the floating pin, to ge a 10-bit unsigned integer
		//save it as a 16bits unsigned integer
		uint16_t reading=analogRead(randomNumPin);
    //get the least significant digit of the integer
		//using bitshift left to put the right most digit to the left most position
		reading= reading << 15;
		//move this digit back to the right most postion
		//other 15 not useful digits are lost during this operation
		reading= reading >> 15;
		str[i]=reading;
		//delay to let the voltage on the pin to fluctuate
		delay(50);
	}
	//end the str array with a null terminator
	str[32]= '\0';
	//convert the 32bits number from array of bits to decimal number
	//initialize the variables used in the calculation
	uint32_t prime=1;
	uint32_t answer=0;
	uint32_t number=0;
	for(int i=0; i<32; i++){
		number=str[i]*prime;
		answer= answer + number;
		prime=prime*2;
	}
	//return the 32bits decimal number
	return answer;
}


//this function uses diffieHellman protocol to get the sharedSecretKey
uint32_t diffieHellman() {
	uint32_t otherPublicKey;
	const uint32_t p = 2147483647;
	const uint32_t g = 16807;

	// get a random 32-bit number as the private key
	// step 1 of setup procedure
	uint32_t myPrivateKey = randomNum();

	// //TEST:
	// Serial.println("should get a 32-bits randomNumber: ");
	// Serial.println(myPrivateKey);
	// Serial.println();

	// step 2 of setup procedure
	// use the power modular calculation function powModFast to get myPublicKey
	uint32_t myPublicKey = powModFast(g, myPrivateKey, p);

	// print the public key to the screen
	// step 3 of setup procedure
	Serial.println("the public key is: ");
	Serial.println(myPublicKey);
	Serial.println();

	// step 4 of setup procedure
	// depends on the reading of idPin
	// using either one of the server and client fsm_client_server
	// automatically exchange publickeys and handshake
	if(digitalRead(idPin) == HIGH){
		Serial.println("I am the server");
		//use the server function to handshake(send myPublicKey, Get otherPublicKey)
		otherPublicKey = server(myPublicKey);
	}
	else if(digitalRead(idPin) == LOW){
		Serial.println("I am the client");
		//use the client function to handshake(send myPublicKey, Get otherPublicKey)
		otherPublicKey = client(myPublicKey);
	}

	Serial.println("otherPublicKey is :");
	Serial.println(otherPublicKey);


	// step 5 of setup procedure
	// use both publickeys to calculate through powMod to get the sharedSecretKey
	uint32_t sharedSecretKey = powModFast(otherPublicKey, myPrivateKey, p);
	return sharedSecretKey;
}

//infinite loop that checks for receiving from the keyboard(Serial1)
uint32_t sender(uint32_t sharedSecretKey){
	while (Serial.available() > 0) {
		// read from keyboard input
		uint8_t character = Serial.read();
		// encrypt using the secret key
		// use XOR encryption

		// // //TEST:
		// Serial.println();
		// Serial.print("rolling key: ");
		// Serial.println((uint8_t)sharedSecretKey);

		uint8_t encryptedChar = character ^ ((uint8_t) sharedSecretKey);
		//print it on user's own monitor
		Serial.write(character);
		//and send the encryptedChar to the receiver
		Serial3.write(encryptedChar);
		//when the enter key is pressed
		if(character == 13){
			//print a line feed to user own monitor
			Serial.write("\n");
			//since a new character '\n' wil be sent
			//we encrypt with a new key
			sharedSecretKey = next_key(sharedSecretKey);

			//send the encryptedlinefeed to the receiver
			uint8_t encryptedlinefeed = 10 ^ ((uint8_t) sharedSecretKey);
			Serial3.write(encryptedlinefeed);
		}
		//rolling key, after we use the key one time
		//we discard it and generate the next key
		sharedSecretKey = next_key(sharedSecretKey);
	}
	return sharedSecretKey;
}

// infinite loop that checks for receiving through Serial3
uint32_t receiver(uint32_t sharedSecretKey){
	while (Serial3.available() > 0) {
		//read from the other arduinos input through Serial3
		char encryptedChar = Serial3.read();

		// decrypt the byte received, and display the corresponding character
		uint8_t decryptedChar = encryptedChar ^ ((uint8_t) sharedSecretKey);
		Serial.write(decryptedChar);
		// the sender sends \r\n with println()
		// so we don't need to add our own \n

		//rolling key, after we use the key one time
		//we discard it and generate the next key
		sharedSecretKey = next_key(sharedSecretKey);
	}
	return sharedSecretKey;
}

/*
The main chat program. Will encrypt and decrypt messages
initiallize a sequence of rolling keys with the sharedSecretKey.
and users can chat by typing
*/

void chat(uint32_t sharedSecretKey) {
	// print some info about the key we are using
	Serial.print("Encrypting with key: ");
	Serial.println(sharedSecretKey);
	Serial.println();
	Serial.print("Cast down to a byte: ");
	Serial.println((uint8_t) sharedSecretKey);
	Serial.println("TYPE to chat");
	Serial.println();
	uint32_t sharedSecretKeyen= sharedSecretKey;
	uint32_t sharedSecretKeyde= sharedSecretKey;
  //starting the infinite loop that checks inputs both from Serial 1 and 3
	while (true) {
		sharedSecretKeyen=sender(sharedSecretKeyen);
		sharedSecretKeyde=receiver(sharedSecretKeyde);
	}
}

int main() {
	setup();

	/* use the randomNum function to obtain the Private Key within
	the diffieHellman function to calculate a sharedSecretKey for encrytion
	purpose and print on monitor to check
	*/
	uint32_t sharedSecretKey = diffieHellman();
	Serial.println("our shared secret key is: ");
	Serial.println(sharedSecretKey);
	Serial.println();

	chat(sharedSecretKey);

	Serial.flush();
	Serial3.flush();

	return 0;
}
