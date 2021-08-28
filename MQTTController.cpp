#include <RF24/RF24.h>
#include <RF24Network/RF24Network.h>
#include <mosquittopp.h>
#include <mosquitto.h>
#include <iostream>
#include <ctime>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Constants that identify nodes
const uint16_t pi_node = 0;
const uint16_t sensor_node1 = 1;
const uint16_t action_node1 = 2;
const uint16_t sensor_node2 = 3;
const uint16_t action_node2 = 4;
const uint16_t sensor_node3 = 5;
const uint16_t sensor_node4 = 6;
const uint16_t sensor_node5 = 7;

const char* action_channel1 = "home/bedroom/light";
const char* action_channel2 = "home/bedroom/";

// CE Pin, CSN Pin, SPI Speed
RF24 radio(RPI_BPLUS_GPIO_J8_15,RPI_BPLUS_GPIO_J8_24, BCM2835_SPI_SPEED_8MHZ);
RF24Network network(radio);

// Time between checking for packets (in ms)
const unsigned long interval = 1000;

// Structure of our messages
struct message_action {
        bool state;
};

struct message_sensor {
        float temperature;
        float humidity;
        bool motion;
        bool lightswitch;
};

struct lightswitch_sensor {
        bool lightswitch;
};

// Mosquitto class
class MyMosquitto : public mosqpp::mosquittopp {
        public:
                MyMosquitto() : mosqpp::mosquittopp ("PiBrain") { mosqpp::lib_init(); }

                virtual void on_connect (int rc) { printf("Connected to Mosquitto\n"); }

                virtual void on_disconnect () { printf("Disconnected\n"); }

                virtual void on_message(const struct mosquitto_message* mosqmessage) {
                        // Message received on a channel we subscribe to
                        printf("Message found on channel %s: %s\n", mosqmessage->topic, mosqmessage->payload);

                        // Create message to send via RF24
                        message_action actionmessage;
                        if (strcmp((char*)mosqmessage->payload, "0") == 0) {
                                actionmessage = (message_action){ false };
                        } else if (strcmp((char*)mosqmessage->payload, "1") == 0) {
                                actionmessage = (message_action){ true };
                        } else {
                                printf("Unknown message: %s\n", mosqmessage->payload);
                                return;
                        }
                        // Determine target node based on channel
                        uint16_t target_node;
                        if (strcmp(mosqmessage->topic, action_channel1) == 0) {
                                target_node = action_node1;
                        } else if (strcmp(mosqmessage->topic, action_channel2) == 0) {
                                target_node = action_node2;
                        }

                        printf("Sending instructions to node %i\n", target_node);

 // Send message on RF24 network
                        RF24NetworkHeader header(target_node);
                        header.type = '2';
                        if (network.write(header, &actionmessage, sizeof(actionmessage))) {
                                printf("Message sent\n");
                        } else {
                                printf("Could not send message\n");
                        }
                }
 };

 MyMosquitto mosq;

int main(int argc, char** argv)
{
        // Initialize all radio related modules
        radio.begin();
        delay(5);
        network.begin(90, pi_node);
 // Print some radio details (for debug purposes)
        radio.printDetails();

        network.update();

        mosq.connect("127.0.0.1");
        mosq.subscribe(0, action_channel1);
        mosq.subscribe(0, action_channel2);

        while (true) {
                // Get the latest network info
                network.update();
                printf(".\n");
                // Enter this loop if there is data available to be read,
                // and continue it as long as there is more data to read
                while ( network.available() ) {
                        RF24NetworkHeader header;
                        message_sensor sensormessage;
                        lightswitch_sensor lightswitchmessage;
                        // Have a peek at the data to see the header type
                        // Have a peek at the data to see the header type
                        network.peek(header);
                        // We can only handle type 1 sensor nodes for now
                        if (header.type == '1') {
                                // Read the message
                                network.read(header, &sensormessage, sizeof(sensormessage));
                                // Print it out in case someone's watching
                                printf("Data received from node %i\n", header.from_node);
                                char buffer [50];
                                switch (header.from_node) {
                                        case sensor_node1:
                                                sprintf (buffer, "mosquitto_pub -t home/bedroom/temperature -m \"%f\"", sensormessage.temperature);
                                                system(buffer);
                                                sprintf (buffer, "mosquitto_pub -t home/bedroom/humidity -m \"%f\"", sensormessage.humidity);
                                                system(buffer);
                                                sprintf (buffer, "mosquitto_pub -t home/bedroom/motion -m \"%i\"", sensormessage.motion ? 1 : 0);
                                                system(buffer);
                                                sprintf (buffer, "mosquitto_pub -t home/bedroom/lightswitch -m \"%i\"", sensormessage.lightswitch ? 1 : 0);
                                                system(buffer);
                                                break;
                                        case sensor_node2:
 sprintf(buffer, "mosquitto_pub -t home/bedroom/lightswitch -m \"%i\"", lightswitchmessage.lightswitch ? 1 : 0);
                                                printf(buffer, "mosquitto_pub -t home/bedroom/lightswitch -m \"%i\"", lightswitchmessage.lightswitch ? 1 : 0);
                                                system(buffer);
 // read other sensor data from node 2 here
                                                break;
                                        // add as many case statements as you have nodes
                                        default:
                                                printf("Unknown node %i\n", header.from_node);
                                                break;
                                }
                        } else if (header.type == '2') {
                                //Read the message
                                network.read(header, &lightswitchmessage, sizeof(lightswitchmessage));
                                // Print it out in case someone's watching
                                printf("Data received from node %i\n", header.from_node);
                                char buffer[50];
                                switch (header.from_node) {
                                        case sensor_node2:
                                                sprintf(buffer, "mosquitto_pub -t home/bedroom/lightswitch -m \"%i\"", lightswitchmessage.lightswitch ? 1 : 0);
                                                printf(buffer, "mosquitto_pub -t home/bedroom/lightswitch -m \"%i\"", lightswitchmessage.lightswitch ? 1 : 0);
                                                system(buffer);
                                                break;
                                        case sensor_node3:
                                                sprintf(buffer, "mosquitto_pub -t home/bedroom/test1/lightswitch -m \"%i\"", lightswitchmessage.lightswitch ? 1 : 0);
                                                printf(buffer, "mosquitto_pub -t home/bedroom/test1/lightswitch -m \"%i\"", lightswitchmessage.lightswitch ? 1 : 0);
                                                system(buffer);
                                                // read other sensor data from node 3 here
                                                break;
                                        case sensor_node4:
                                                sprintf(buffer, "mosquitto_pub -t home/bedroom/test2/lightswitch -m \"%i\"", lightswitchmessage.lightswitch ? 1 : 0);
                                                printf(buffer, "mosquitto_pub -t home/bedroom/test2/lightswitch -m \"%i\"", lightswitchmessage.lightswitch ? 1 : 0);
                                                system(buffer);
                                                // read other sensor data from node 4 here
                                                break;
                                        case sensor_node5:
                                                sprintf(buffer, "mosquitto_pub -t home/bedroom/test3/lightswitch -m \"%i\"", lightswitchmessage.lightswitch ? 1 : 0);
                                                printf(buffer, "mosquitto_pub -t home/bedroom/test3/lightswitch -m \"%i\"", lightswitchmessage.lightswitch ? 1 : 0);
                                                system(buffer);
                                                // read other sensor data from node 5 here
                                                break;
                                        // add as many case statements as you have nodes running light switch sensor
 default:
                                        printf("Unknown node %i\n", header.from_node);
                                        break;
                                }
                        } else {
                                // This is not a type we recognize
                                network.read(header, &sensormessage, sizeof(sensormessage));
                                printf("Unknown message received from node %i\n", header.from_node);
                        }
                }

                // Check for messages on our subscribed channels
                mosq.loop();

                delay(interval);
        }
 mosq.unsubscribe(0, action_channel1);
        mosq.unsubscribe(0, action_channel2);

        // last thing we do before we end things
        return 0;
}