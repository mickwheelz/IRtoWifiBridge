# IRtoWiFiBridge

a simple IR to WiFi bridge for home automation using ESP8266 and MQTT

## Description

This sketch will publish IR codes it recieves to `irsend/recv` in JSON format and will blast any codes it recieves on `irsend/send`

Codes are sent/recieved in the below JSON format via MQTT. The list of manufacturers is per the IR library here: https://github.com/markszabo/IRremoteESP8266

```
{"manufacturer":"SAMSUNG","code":3772833823,"bits":32}
```

## Hardware

You can use any ESP8266 board with an IR LED and IR Reciever (e.g TSOP38238)

I am using a NodeMCU board with the IR LED on Pin D2 and the Reciever on pin D6.
