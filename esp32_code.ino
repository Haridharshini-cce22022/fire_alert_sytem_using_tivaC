#include <TinyGPS++.h>
#include <WiFi.h>
#include <ESP_Mail_Client.h>
#include <HardwareSerial.h>

// Define the GPS module connection pins for ESP32
#define GPS_TX_PIN 17  // Connect to RX pin of GPS module
#define GPS_RX_PIN 16  // Connect to TX pin of GPS module
#define TRIGGER_PIN 4  // Pin for external trigger

// Your WiFi credentials
#define WIFI_SSID "Your device ssid"
#define WIFI_PASSWORD "Your wifi password"

// Your email credentials and recipient details
#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT 587
#define AUTHOR_EMAIL "Your gmail ID"
#define AUTHOR_PASSWORD "gmail app password"
#define RECIPIENT_EMAIL "Recipient gmil ID"

// Create an object of the class TinyGPSPlus
TinyGPSPlus gps;

// Define HardwareSerial for GPS communication
HardwareSerial GPS_Serial(2); // Using UART2

// Declare the global used SMTPSession object for SMTP transport
SMTPSession smtp;

// Declare the session configuration
Session_Config config;

// Function declarations
void sendEmail(double lat, double lng);
void smtpCallback(SMTP_Status status);
static void smartDelay(unsigned long ms);

void setup() {
  Serial.begin(9600); // Define baud rate for serial communication
  GPS_Serial.begin(9600, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN); // Initialize hardware serial port for GPS communication

  pinMode(TRIGGER_PIN, INPUT);

  // Connect to Wi-Fi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  // Set the network reconnection option
  MailClient.networkReconnect(true);

  // Enable the debug via Serial port
  smtp.debug(2);

  // Set the callback function to get the sending results
  smtp.callback(smtpCallback);

  // Set the session config
  config.server.host_name = SMTP_HOST;
  config.server.port = SMTP_PORT;
  config.login.email = AUTHOR_EMAIL;
  config.login.password = AUTHOR_PASSWORD;
  config.login.user_domain = "";

  // Set the NTP config time
  config.time.ntp_server = F("pool.ntp.org,time.nist.gov");
  config.time.gmt_offset = 5.5;
  config.time.day_light_offset = 0;
}

void loop() {
  if (digitalRead(TRIGGER_PIN) == HIGH) {
    // Wait for GPS data
    smartDelay(1000);
    if (gps.location.isValid()) {
      double lat = gps.location.lat();
      double lng = gps.location.lng();
      sendEmail(lat, lng);
    } else {
      Serial.println("No valid GPS data");
    }

    // Wait until the trigger signal goes LOW
    while (digitalRead(TRIGGER_PIN) == HIGH) {
      delay(100);
    }
  }

  delay(100);
}

void sendEmail(double lat, double lng) {
  // Declare the message class
  SMTP_Message message;

  // Set the message headers
  message.sender.name = "FIRE ALERT SYSTEM";
  message.sender.email = AUTHOR_EMAIL;
  message.subject = "CAUTUION : FIRE DETECTED";
  message.addRecipient("Recipient", RECIPIENT_EMAIL);

  // Create Google Maps link
  String gmapLink = "https://maps.google.com/?q=" + String(lat, 6) + "," + String(lng, 6);
  String textMsg = "Fire detected Location: " + gmapLink;

  message.text.content = textMsg.c_str();
  message.text.charSet = "us-ascii";
  message.text.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

  message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;
  message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;

  // Connect to the server
  if (!smtp.connect(&config)) {
    Serial.printf("Connection error, Status Code: %d, Error Code: %d, Reason: %s\n", smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());
    return;
  }

  // Send the email and close the session
  if (!MailClient.sendMail(&smtp, &message)) {
    Serial.printf("Error sending email, Status Code: %d, Error Code: %d, Reason: %s\n", smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());
  } else {
    Serial.println("Email sent successfully");
  }

  // Clear the message for the next use
  message.text.content = "";
}

void smtpCallback(SMTP_Status status) {
  // Print the current status
  Serial.println(status.info());

  // Print the sending result
  if (status.success()) {
    Serial.println("----------------");
    Serial.printf("Message sent success: %d\n", status.completedCount());
    Serial.printf("Message sent failed: %d\n", status.failedCount());
    Serial.println("----------------\n");

    for (size_t i = 0; i < smtp.sendingResult.size(); i++) {
      // Get the result item
      SMTP_Result result = smtp.sendingResult.getItem(i);
      Serial.printf("Message No: %d\n", i + 1);
      Serial.printf("Status: %s\n", result.completed ? "success" : "failed");
      Serial.printf("Date/Time: %s\n", MailClient.Time.getDateTimeString(result.timestamp, "%B %d, %Y %H:%M:%S").c_str());
      Serial.printf("Recipient: %s\n", result.recipients.c_str());
      Serial.printf("Subject: %s\n", result.subject.c_str());
    }
    Serial.println("----------------\n");

    // Clear sending result to free up memory
    smtp.sendingResult.clear();
  }
}

static void smartDelay(unsigned long ms) {
  unsigned long start = millis();
  do {
    while (GPS_Serial.available()) // Encode data read from GPS while data is available on serial port
      gps.encode(GPS_Serial.read());
  } while (millis() - start < ms);
}
