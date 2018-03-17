/*
Name: LIYAO JIANG(1512446)
Section#: EA1

Name：XIAOLEI ZHANG（1515335）
Section#：LBL A1
*/

#include <Arduino.h>
#include "fsm_client_server.h"

//wait_on_serial3,uint32_to_serial3,uint32_from_serial3
//are all copied from eclass(assignment discription)

/** Waits for a certain number of bytes on Serial3 or timeout
 * @param nbytes: the number of bytes we want
 * @param timeout: timeout period (ms); specifying a negative number
 *                turns off timeouts (the function waits indefinitely
 *                if timeouts are turned off).
 * @return True if the required number of bytes have arrived.
 */
bool wait_on_serial3( uint8_t nbytes, long timeout ) {
  unsigned long deadline = millis() + timeout;//wraparound not a problem
  while (Serial3.available()<nbytes && (timeout<0 || millis()<deadline))
  {
    delay(1); // be nice, no busy loop
  }
  return Serial3.available()>=nbytes;
}

/** Writes an uint32_t to Serial3, starting from the least-significant
 * and finishing with the most significant byte.
 */
void uint32_to_serial3(uint32_t num) {
  Serial3.write((char) (num >> 0));
  Serial3.write((char) (num >> 8));
  Serial3.write((char) (num >> 16));
  Serial3.write((char) (num >> 24));
}

/** Reads an uint32_t from Serial3, starting from the least-significant
 * and finishing with the most significant byte.
 */
uint32_t uint32_from_serial3() {
  uint32_t num = 0;
  num = num | ((uint32_t) Serial3.read()) << 0;
  num = num | ((uint32_t) Serial3.read()) << 8;
  num = num | ((uint32_t) Serial3.read()) << 16;
  num = num | ((uint32_t) Serial3.read()) << 24;
  return num;
}

//We use two FSM to handshake and exchange keys

//the FSM for the client
uint32_t client(uint32_t ckey){
  uint32_t skey;
  enum State { Start, WaitingForAck, DataExchange };
  State currentState = Start;
  while (true) {
    // FSM control
    if (currentState == Start){
      //send 'C' followed by 32bit client key
      Serial3.write('C');
      uint32_to_serial3(ckey);
      currentState = WaitingForAck;
    }
    //if timeout go back to Start
    else if ((currentState == WaitingForAck) && (!wait_on_serial3( 5,1000 )) ){
      currentState = Start;
    }

    //Whether or not Received Ack?
    //if waiting for  Ack and 5bytes arrived and didn't timeout
    else if ((currentState == WaitingForAck) && (wait_on_serial3( 5,1000 ))){
      //check for the ACK
      uint8_t ACK = Serial3.read();
      if(ACK == 'A'){
        //store the server key
        skey= uint32_from_serial3();
        //client sends the acknowledgement. before DataExchange
        Serial3.write('A');
        currentState = DataExchange;
      }
      else {currentState = WaitingForAck;}
    }

    else if (currentState == DataExchange){break;}
  }
  return skey;
}

//the FSM for the server
uint32_t server(uint32_t skey){
  uint32_t ckey;
  enum State { Listen, WaitingForKey1, WaitForAck1, WaitingForKey2,WaitForAck2,DataExchange };
  State currentState = Listen;
  while (true) {
    // FSM control
    //Listen
    if (currentState == Listen){
      uint8_t byte = Serial3.read();
      if (byte == 'C'){
        currentState = WaitingForKey1;
      }
    }

    //WaitingForKey1
    //if timeout go back to Start
    else if ((currentState == WaitingForKey1) && (!wait_on_serial3( 4,1000 )) ){
      currentState = Listen;

    }
    else if ((currentState == WaitingForKey1) && (wait_on_serial3( 4,1000 ))){
      //store the client key
      ckey= uint32_from_serial3();
      Serial.println(ckey);
      //send ACK(skey)
      Serial3.write('A');
      uint32_to_serial3(skey);
      currentState = WaitForAck1;
    }

    //WaitForAck1
    //if timeout, go to listen
    else if ((currentState == WaitForAck1) && (!wait_on_serial3( 1,1000))){
      currentState = Listen;
    }
    else if ((currentState == WaitForAck1) && (wait_on_serial3( 1,1000))){
      uint8_t byte = Serial3.read();
      //When receiving C
      if( byte == 'C'){
        currentState = WaitingForKey2;
      }
      //when receiving A
      else if( byte == 'A'){
        currentState = DataExchange;
      }
    }

    //WaitingForKey2
    //if timeout
    else if ((currentState == WaitingForKey2) && (!wait_on_serial3( 4,1000 ))){
      currentState = Listen;
    }
    else if ((currentState == WaitingForKey2) && (wait_on_serial3( 4,1000 ))){
      //store the client key
      ckey= uint32_from_serial3();
      currentState = WaitForAck2;
    }

    //WaitForAck2
    //if timeout
    else if ((currentState == WaitForAck2) && (!wait_on_serial3( 1,1000))){
      currentState = Listen;
    }
    //same as WaitForAck1
    else if ((currentState == WaitForAck2) && (wait_on_serial3( 1,1000))){
      uint8_t byte = Serial3.read();
      if( byte == 'C'){
        currentState = WaitingForKey2;
      }
      else if( byte == 'A'){
        currentState = DataExchange;
      }
    }

    //DataExchange
    else if (currentState == DataExchange){break;}
  }
  return ckey;
}
