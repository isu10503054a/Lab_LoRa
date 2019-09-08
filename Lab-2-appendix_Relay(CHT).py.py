import client as mqtt
import paho.mqtt.publish as publish
import json
import time
import urllib.parse
import urllib.request

GIOT_ULTopic_prefix = "GIOT-GW/UL/"      # MQTT topic for Uplink
GW_MAC = "1C497BF27096"                  # LoRa gateway MACaddr
Target_node_MAC = "0000000099999999"     # LoRa node MACaddr

# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    print("Connected with result code " + str(rc))
    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    client.subscribe(GIOT_ULTopic_prefix + GW_MAC)


# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):

    json_extractor = json.loads(msg.payload)
    print(msg.topic+" "+str(msg.payload))
    if json_extractor[0]['macAddr'] == Target_node_MAC:
        string_value = json_extractor[0]['data']

        #print(int (string_value))
        if int(string_value) > 1:
            #print(string_value)
            #print(string_value[0:2] + "." + string_value[2:4])
            #print(string_value[4:6] + "." + string_value[6:8])
            temperature = string_value[0:2] + "." + string_value[2:4]
            humidity = string_value[4:6] + "." + string_value[6:8]
            whattime = time.strftime("%Y-%m-%dT%H:%M:%S", time.localtime())
            CHT_data_format =' [{ "id": "temperature", "time": "' + whattime + '","value": [ "' + temperature + '"]},{"id": "humidity", "time": "' + whattime + '", "value": ["' + humidity + '"]}]'
            print("CHT_data_format : ")
            print(CHT_data_format)

            #CHT IoT smart platform INFO
            host = "iot.cht.com.tw"
            auth = {'username': "DKMCSBWRM0RWPC2XMT", 'password': "DKMCSBWRM0RWPC2XMT"}         #Device key(username & password is same)
            topic = "/v1/device/18414704244/rawdata"                                 #/v1/device/ (Device number) /rawdata
            publish.single(topic, CHT_data_format, qos=0, hostname=host, auth=auth)


# Connect MQTT info
client = mqtt.Client(client_id="", protocol=mqtt.MQTTv31)
client.on_connect = on_connect
client.on_message = on_message
client.username_pw_set("", password="")
client.connect("140.127.196.98", port=1883, keepalive=60)  # MQTT IP address
client.loop_forever()