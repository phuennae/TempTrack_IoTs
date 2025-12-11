# TempTrack_IoTs
Hardware code for Blockchain and IoTs in supply chain management


How to use ESP32 & EC2 server

Step 1 : Log-in in to AWS amazon website and go to EC2 dashboard.

Step 2 : Click instance and click clear filters.

Step 3 : Select server name "Temp_Track"

Step 4 : Click "Instance start" and select "Start instance"

Step 5 : Click "Connect".

Step 6 : Open Aduino IDE and Paste the code from "Temtrack_MQTT.ino".

Step 7 : Change the WIFI username & password.

Step 8 : !Change the IPv4 address of EC2 server! (Important)

Step 9 : Burn code in to ESP32.

Step 10 : Go to Ubuntu in EC2 and type "mosquitto_sub -h localhost -t "#" -v" to show all data that server recieved.
 
