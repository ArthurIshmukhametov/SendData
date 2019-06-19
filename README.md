# SendData
## Libraries
1. AsyncMqttClient
2. EtherCard
3. HTTPClient.h
4. WiFi.h

*Unimportant*

5. *AsyncTCP*
6. *Ethernet*
7. *UIPEthernet*
8. *Arduino Timer Api*

#include <>
Send data from ESP32 via WiFi-HTTP/WiFi-MQTT/Ethernet-HTTP

Данный скетч позволяет отправлять данные на сервер по одному из трёх каналов: WiFi протокол HTTP, WiFi протокол MQTT, Ethernet протокол HTTP.
Выбор канала отправки производится автоматически или же задаётся вручную по приоритетам.

Данные хранятся в формате словара, где ключём является топик.
*******************************
# SendData
This scetch allows send data to server via one of these chanels: WiFi-HTTP/WiFi-MQTT/Ethernet-HTTP. Chanel choosing is fully automized, but you can set prioritets.

Дата is being stored as a dictionary, with a topic as a key
