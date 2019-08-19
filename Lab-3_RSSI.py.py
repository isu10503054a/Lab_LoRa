import client as mqtt
import json

GIOT_ULTopic_prefix = "GIOT-GW/UL/"
LAN_MAC = "1C497BF27096"                                    # gw位置(規定的mac位置,下面也同)
Target_node_MAC = "0000000099999999"                        #LoRa node macaddr

# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))
    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    client.subscribe(GIOT_ULTopic_prefix + LAN_MAC)

# Show message.
def on_message(client, userdata, msg):
    json_extractor = json.loads(msg.payload)
    if json_extractor[0]['macAddr'] == Target_node_MAC:
        print("\n Subscribe message:")
        print(msg.topic + " " + str(msg.payload))


client = mqtt.Client(client_id="", protocol=mqtt.MQTTv31)
client.on_connect = on_connect
client.on_message = on_message
client.username_pw_set("", password="")
client.connect("140.127.196.98", port=1883, keepalive=60)                                        #MQTTbroker位址

# Blocking call that processes network traffic, dispatches callbacks and
# handles reconnecting.
# Other loop*() functions are available that give a threaded interface and a
# manual interface.

if __name__ == '__main__':
    client.loop_forever()
