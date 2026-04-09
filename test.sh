#!/bin/bash

# Параметры запуска
UE_BIN="./build/ue"
ARGS="-p 8085 -i 0.0.0.0 -imei 158863118273320 -imsi 12345601112233 -msisdn 89991112233 -x 10"
MY_MSISDN="89993334455"

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