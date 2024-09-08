# fire_alert_sytem_using_tivaC
This project outlines the development of a smart 
fire alarm system leveraging the Tiva C microcontroller, a 
flame sensor, a buzzer, the ESP32 module, and a GPS module. 
The system is designed to detect fire through connected sensor, 
introduced a five-second delay to avoid false alarms, and 
subsequently trigger a buzzer for local alerts while also 
sending an email notification for remote alerts. Additionally, 
the GPS module is integrated to determine the longitude and 
latitude of the current location, which is included in the email 
notification. The Tiva C processes sensor data, manages the 
delay, and communicates with the ESP32, which handles Wi-Fi 
connectivity, email dispatch, and retrieves location data from 
the GPS module. This comprehensive system enhances fire 
safety by providing immediate on-site alerts and detailed 
remote notifications, ensuring rapid response and protection 
for both residential and commercial environments. Key 
features include reliable communication between components, 
and a mechanism to reduce false alarms.
