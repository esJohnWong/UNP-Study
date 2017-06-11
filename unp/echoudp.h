#pragma once

#include <string>

void echoserver_udp();
void echocli_udp(const std::string &servAddrStr);
void echocli_udp_connect(const std::string &servAddrStr);

void echocli_udp_flow_test(const std::string &servAddrStr);

void echoserver_udp_flew_test();