// This example uses an Adafruit Huzzah ESP8266
// to connect to shiftr.io.
//
// You can check on your device after a successful
// connection here: https://shiftr.io/try.
//
// by Joël Gähwiler
// https://github.com/256dpi/arduino-mqtt
//
// Updated to include Secure Connections by BSG_Engineering

#include <ESP8266WiFi.h>
#include <MQTT.h>
#include <time.h>                 

const char ssid[] = "";        //WiFi SSID
const char pass[] = "";               //WiFi Password

//populate the below variables if you would like to define everything up here

const char clientId[] = "";           //MQTT ClientID  
const char username[] = "";           //MQTT Username (if any)
const char password[] = "";           //MQTT Password (if any)

static const char fp[] PROGMEM = "";  //SHA1 Fingerprint
const char *   host = "";             //MQTT Host Address
const uint16_t port = 8883;
const char *   path = "/";

WiFiClientSecure net;
MQTTClient client;

unsigned long lastMillis = 0;

void connect() {
  if(WiFi.status() != WL_CONNECTED && (!client.connect(clientId, username, password) || !client.connect("arduino", "try", "try"))){
  Serial.print("checking wifi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }

 Serial.println("WiFi Connected!");

 Serial.println("\nconnecting to MQTT...");
 
  // do not verify tls certificate
  // check the following example for methods to verify the server:
  // https://github.com/esp8266/Arduino/blob/master/libraries/ESP8266WiFi/examples/BearSSL_Validation/BearSSL_Validation.ino
    net.setInsecure();      //ONLY UNCOMMENT ONE OF THESE OPTIONS!(insecure, SHA1, Key OR Cert)
  //fetchFingerprint();     //use SHA1 Fingerprint for Secure connection
  //fetchKnownKey();        //use KEY for Secure connection
  //fetchCertAuthority();   //use Cert for Secure connection

  Serial.print("\nconnecting...");
  while (!client.connect("arduino", "try", "try")) {
    Serial.print(".");
    delay(1000);

//  while (!client.connect(clientId, username, password)) {     //Uncomment if you would like to use
//   Serial.print(".");                                         //defined variables above for client, user, password
//   delay(1000);
    
  }

 Serial.println("\nconnected to MQTT!");

     client.subscribe("/hello");
  // client.subscribe("/hello", 2);     //topic "hello", QoS 2
  // client.unsubscribe("/hello");
  }
}

void messageReceived(String &topic, String &payload) {
  Serial.println("incoming: " + topic + " - " + payload);
}

void fetchURL(BearSSL::WiFiClientSecure *net, const char *host, const uint16_t port, const char *path) {
 if (!path) {
   path = "/";
 }

 Serial.printf("Trying: %s:8883...", host);
 net->connect(host, port);
 if (!net->connected()) {
   Serial.printf("*** Can't connect. ***\n-------\n");
   return;
 }
 Serial.printf("Connected!\n-------\n");
}

void fetchFingerprint() {
 Serial.println("Verifying Fingerprint (SHA1)");
 net.setFingerprint(fp);
 fetchURL(&net, host, port, path);           //Must use defined variables above to pass correct data
}


void fetchKnownKey() {
 Serial.printf(R"EOF(
The server certificate can be completely ignored and its public key
hardcoded in your application. This should be secure as the public key
needs to be paired with the private key of the site, which is obviously
private and not shared.  A MITM without the private key would not be
able to establish communications.
)EOF");
 // Extracted by: openssl x509 -pubkey -noout -in servercert.pem
 // Update the KEY to match your private key
 static const char pubkey[] PROGMEM = R"KEY(
-----BEGIN PUBLIC KEY-----
MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAy+3Up8qBkIn/7S9AfWlH
Od8SdXmnWx+JCIHvnWzjFcLeLvQb2rMqqCDL5XDlvkyC5SZ8ZyLITemej5aJYuBv
zcKPzyZ0QfYZiskU9nzL2qBQj8alzJJ/Cc32AWuuWrPrzVxBmOEW9gRCGFCD3m0z
53y6GjcmBS2wcX7RagqbD7g2frEGko4G7kmW96H6dyh2j9Rou8TwAK6CnbiXPAM/
5Q6dyfdYlHOCgP75F7hhdKB5gpprm9A/OnQsmZjUPzy4u0EKCxE8MfhBerZrZdod
88ZdDG3CvTgm050bc+lGlbsT+s09lp0dgxSZIeI8+syV2Owt4YF/PdjeeymtzQdI
wQIDAQAB
-----END PUBLIC KEY-----
)KEY";
 
 BearSSL::PublicKey key(pubkey);
 net.setKnownKey(&key);
 fetchURL(&net, host, port, path);             //Must use defined variables above to pass correct data
}

void fetchCertAuthority() {
 static const char digicert[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDxTCCAq2gAwIBAgIQAqxcJmoLQJuPC3nyrkYldzANBgkqhkiG9w0BAQUFADBs
MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3
d3cuZGlnaWNlcnQuY29tMSswKQYDVQQDEyJEaWdpQ2VydCBIaWdoIEFzc3VyYW5j
ZSBFViBSb290IENBMB4XDTA2MTExMDAwMDAwMFoXDTMxMTExMDAwMDAwMFowbDEL
MAkGA1UEBhMCVVMxFTATBgNVBAoTDERpZ2lDZXJ0IEluYzEZMBcGA1UECxMQd3d3
LmRpZ2ljZXJ0LmNvbTErMCkGA1UEAxMiRGlnaUNlcnQgSGlnaCBBc3N1cmFuY2Ug
RVYgUm9vdCBDQTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAMbM5XPm
+9S75S0tMqbf5YE/yc0lSbZxKsPVlDRnogocsF9ppkCxxLeyj9CYpKlBWTrT3JTW
PNt0OKRKzE0lgvdKpVMSOO7zSW1xkX5jtqumX8OkhPhPYlG++MXs2ziS4wblCJEM
xChBVfvLWokVfnHoNb9Ncgk9vjo4UFt3MRuNs8ckRZqnrG0AFFoEt7oT61EKmEFB
Ik5lYYeBQVCmeVyJ3hlKV9Uu5l0cUyx+mM0aBhakaHPQNAQTXKFx01p8VdteZOE3
hzBWBOURtCmAEvF5OYiiAhF8J2a3iLd48soKqDirCmTCv2ZdlYTBoSUeh10aUAsg
EsxBu24LUTi4S8sCAwEAAaNjMGEwDgYDVR0PAQH/BAQDAgGGMA8GA1UdEwEB/wQF
MAMBAf8wHQYDVR0OBBYEFLE+w2kD+L9HAdSYJhoIAu9jZCvDMB8GA1UdIwQYMBaA
FLE+w2kD+L9HAdSYJhoIAu9jZCvDMA0GCSqGSIb3DQEBBQUAA4IBAQAcGgaX3Nec
nzyIZgYIVyHbIUf4KmeqvxgydkAQV8GK83rZEWWONfqe/EW1ntlMMUu4kehDLI6z
eM7b41N5cdblIZQB2lWHmiRk9opmzN6cN82oNLFpmyPInngiK3BD41VHMWEZ71jF
hS9OMPagMRYjyOfiZRYzy78aG6A9+MpeizGLYAiJLQwGXFK3xPkKmNEVX58Svnw2
Yzi9RKR/5CYrCsSXaQ3pjOLAEFe4yHYSkVXySGnYvCoCWw9E1CAx2/S6cCZdkGCe
vEsXCS+0yx5DaMkHJ8HSXPfqIbloEpw8nL+e/IBcm2PN7EeqJSdnoDfzAIJ9VNep
+OkuE6N36B9K
-----END CERTIFICATE-----
)EOF";

 Serial.printf(R"EOF(
A specific certification authority can be passed in and used to validate
a chain of certificates from a given server.  These will be validated
using BearSSL's rules, which do NOT include certificate revocation lists.
A specific server's certificate, or your own self-signed root certificate
can also be used.  ESP8266 time needs to be valid for checks to pass as
BearSSL does verify the notValidBefore/After fields.
)EOF");

 BearSSL::X509List cert(digicert);
 net.setTrustAnchors(&cert);
 Serial.printf("Try validating without setting the time (should fail)\n");
 fetchURL(&net, host, port, path);          //Must use defined variables above to pass correct data

 Serial.printf("Try again after setting NTP time (should pass)\n");
 setClock();
 fetchURL(&net, host, port, path);          //Must use defined variables above to pass correct data
}

// Set time via NTP, as required for x.509 validation
void setClock() {
 configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");

 Serial.print("Waiting for NTP time sync: ");
 time_t now = time(nullptr);
 while (now < 8 * 3600 * 2) {
   delay(500);
   Serial.print(".");
   now = time(nullptr);
 }
 Serial.println("");
 struct tm timeinfo;
 gmtime_r(&now, &timeinfo);
 Serial.print("Current time: ");
 Serial.print(asctime(&timeinfo));
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, pass);

  // Note: Local domain names (e.g. "Computer.local" on OSX) are not supported by Arduino.
  // You need to set the IP address directly.
  //
  // MQTT brokers usually use port 8883 for secure connections.
  client.begin("broker.shiftr.io", 8883, net);
  //client.begin(host, port, net);        //use defined variables above
  client.onMessage(messageReceived);

  connect();
}

void loop() {
  client.loop();
  delay(10);  // <- fixes some issues with WiFi stability

  if (!client.connected()) {
    connect();
  }

  // publish a message roughly every second.
  if (millis() - lastMillis > 1000) {
    lastMillis = millis();
    client.publish("/hello", "world");
  }
}
