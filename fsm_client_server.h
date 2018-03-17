/*
Name: LIYAO JIANG(1512446)
Section#: EA1

Name：XIAOLEI ZHANG（1515335）
Section#：LBL A1
*/
#ifndef _FSM_CLIENT_SERVER_H_
#define _FSM_CLIENT_SERVER_H_

// function declarations here
// the definitions (implementations) are in fsm_client_server.cpp
bool wait_on_serial3( uint8_t nbytes, long timeout );
void uint32_to_serial3(uint32_t num);
uint32_t uint32_from_serial3();

uint32_t client(uint32_t ckey);
uint32_t server(uint32_t skey);

#endif
