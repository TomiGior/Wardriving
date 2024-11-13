#!/bin/bash

sync

#Flush previous log.
printf "═══════════════════════════════════════════════════════════\nBUTTON PRESSED: NEW SESSION ON $(date)\n═══════════════════════════════════════════════════════════\n">/root/log.txt

#Kill wireless processes and put interfaces in monitor mode:
printf "═════════════════════════════\n$(date +"%H:%M:%S"): AIRMON CHECK KILL\n═════════════════════════════\n"
airmon-ng check kill &>>/root/log.txt

printf "════════════════════════════\n$(date +"%H:%M:%S"): AIRMON START wlan0\n════════════════════════════\n">>/root/log.txt
airmon-ng start wlan0 &>>/root/log.txt

printf "══════════════════════════════\n$(date +"%H:%M:%S"): AIRMON START wlan0-1\n══════════════════════════════\n">>/root/log.txt
airmon-ng start wlan0-1 &>>/root/log.txt

printf "════════════════════════════\n$(date +"%H:%M:%S"): AIRMON START wlan1\n════════════════════════════\n">>/root/log.txt
airmon-ng start wlan1 &>>/root/log.txt

printf "════════════════════════════\n$(date +"%H:%M:%S"): AIRMON START wlan2\n════════════════════════════\n">>/root/log.txt
airmon-ng start wlan2 &>>/root/log.txt

#Activate GPSD service:
printf "═══════════════════════\n$(date +"%H:%M:%S"): ACTIVATE GPSD\n═══════════════════════\n">>/root/log.txt
gpsd /dev/ttyACM0

#Turn on Kismet in Wardriving mode:
printf "═══════════════════════════════\n$(date +"%H:%M:%S"): TURN-ON Kismet server\n═══════════════════════════════\n">>/root/log.txt
kismet -c wlan0mon -c wlan0-1mon -c wlan1mon -c wlan2mon --override wardrive 

#Enjoy!!!
#S731N
