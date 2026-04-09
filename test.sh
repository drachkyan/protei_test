#!/bin/bash

# Параметры запуска
UE_BIN="./build/ue"
ARGS="-p 8085 -i 0.0.0.0 -imei 325314891006270 -imsi 12345603334459 -msisdn 89993334459 -x 10"
MY_MSISDN="89993334458"

{
  sleep 0.1
  echo "activate"

  sleep 0.1

  for i in {1..50}
  do
    echo "send_message"
    sleep 0.01
    echo "$MY_MSISDN"
    sleep 0.01
    echo "Message number $i"
    sleep 0.01
  done
  sleep 1
  echo "exit"

} | $UE_BIN $ARGS