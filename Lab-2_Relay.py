import requests
import socket
import threading
import logging
import client as mqtt
import json
#import urllib.request

GIOT_ULTopic_prefix = "GIOT-GW/UL/"                        # MQTT topic for Uplink
GW_MAC = "1C497BF27096"                                    # LoRa gateway MACaddr
Target_node_MAC = "0000000099999999"                       # LoRa node MACaddr

# JSON format to MCS web console
# if MCS Data channel Id is different please change "dataChnId" value
mcs_data_format = {
    "datapoints": [
        {
            "dataChnId": "lora_temp",
            "values": {
                "value": "00.00"
            }
        },
        {
            "dataChnId": "lora_humi",
            "values": {
                "value": "00.00"
            }
        }
    ]
}

# change this to the values from MCS web console
DEVICE_INFO = {
    'device_id': 'DQia0la8',
    'device_key': 'dbHGtos8wx8Xfj1i'
}

# change 'INFO' to 'WARNING' to filter info messages
logging.basicConfig(level='INFO')

heartBeatTask = None
device_status = 0

def establishCommandChannel():
    # Query command server's IP & port
    connectionAPI = 'https://api.mediatek.com/mcs/v2/devices/%(device_id)s/connections.csv'
    r = requests.get(connectionAPI % DEVICE_INFO,
                 headers = {'deviceKey' : DEVICE_INFO['device_key'],
                            'Content-Type' : 'text/csv'})
    logging.info("Command Channel IP,port=" + r.text)
    (ip, port) = r.text.split(',')

    # Connect to command server
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((ip, int(port)))
    s.settimeout(None)

    # Heartbeat for command server to keep the channel alive
    def sendHeartBeat(commandChannel):
        keepAliveMessage = '%(device_id)s,%(device_key)s,0' % DEVICE_INFO
        commandChannel.sendall(keepAliveMessage.encode(encoding='utf_8', errors='strict'))
        logging.info("beat:%s" % keepAliveMessage)
        # check the value - it's either 0 or 1

    def heartBeat(commandChannel):
        sendHeartBeat(commandChannel)
        # Re-start the timer periodically
        global heartBeatTask
        heartBeatTask = threading.Timer(40, heartBeat, [commandChannel]).start()

    heartBeat(s)
    return s

# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))
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
            mcs_data_format['datapoints'][0]['values']['value'] = string_value[0:2] + "." + string_value[2:4]
            mcs_data_format['datapoints'][1]['values']['value'] = string_value[4:6] + "." + string_value[6:8]
            print(mcs_data_format)

            url = "http://api.mediatek.com/mcs/v2/devices/" +  DEVICE_INFO['device_id'] + "/datapoints"
            headers = {"Content-type": "application/json", "deviceKey": DEVICE_INFO['device_key']}

            post_to_mcs = requests.post(url, headers=headers, data=json.dumps(mcs_data_format))
            #print(post_to_mcs.url)


# Connect MQTT info
client = mqtt.Client(client_id="", protocol=mqtt.MQTTv31)
client.on_connect = on_connect
client.on_message = on_message
client.username_pw_set("", password="")
client.connect("140.127.196.98", port=1883, keepalive=60)                # MQTT IP address


if __name__ == '__main__':
    channel = establishCommandChannel()
    client.loop_forever()
    while (True):
        pass