import requests
import socket
import threading
import _thread
import logging
import time
import client as mqtt
import json
#import urllib.request

GIOT_ULTopic_prefix = "GIOT-GW/UL/"                        # MQTT topic for Uplink
GIOT_DLTopic_prefix = "GIOT-GW/DL/"                        # MQTT topic for Downlink
GW_MAC = "1C497BF27096"                                    # gateway MACaddr
GW_ID = "00001c497beba4bc"                                 # gateway ID
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
downlink_data = [{
    "macAddr": "0000000000000000",
    "data": "0000",
    "id": "998877abcd0123",
    "extra": {
        "port": 2,
        "txpara": 2
    }
}]

# change this to the values from MCS web console
DEVICE_INFO = {
    'device_id': 'DQia0la8',
    'device_key': 'dbHGtos8wx8Xfj1i'
}

# change 'INFO' to 'WARNING' to filter info messages
logging.basicConfig(level='INFO')
heartBeatTask = None

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


device_status = 0
def waitAndExecuteCommand(commandChannel):
    global dlmsg
    global device_status
    led_status = '00'
    fan_status = '00'

    while True:
        command = str(commandChannel.recv(1024))
        logging.info("recv:" + command)
        # command can be a response of heart beat or an update of the LED_control,
        # so we split by ',' and drop device id and device key and check length
        fields = command.strip().split(',' )[2:]

        if len(fields) > 1:
            timeStamp, dataChannelId, commandString = fields
            if dataChannelId == 'lora_led':
                commandString = commandString[:-1]
                led_status = "0" + str(commandString)
                logging.info("led :" + commandString)
                dlmsg = 1
            elif dataChannelId == 'lora_fan':
                commandString = commandString[:-1]
                fan_status = "0" + str(commandString)
                logging.info("fan :" + commandString)
                dlmsg = 1

        device_status = str(led_status) + str(fan_status)
        logging.info("Device_status :" + device_status)
        downlink_data[0]['data'] = device_status
        #print ( downlink_data[0]['data'] )



# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))
    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    client.subscribe(GIOT_ULTopic_prefix + GW_MAC)

# The callback for when a PUBLISH message is received from the server,
# PUBLISH Downlink message when message is received.
def on_message(client, userdata, msg):

    json_extractor = json.loads(msg.payload)
    print(msg.topic+" "+str(msg.payload))
    if json_extractor[0]['macAddr'] == Target_node_MAC:
        string_value = json_extractor[0]['data']

        #print(int (string_value))
        if int(string_value) > 1:
            mcs_data_format['datapoints'][0]['values']['value'] = string_value[0:2] + "." + string_value[2:4]
            mcs_data_format['datapoints'][1]['values']['value'] = string_value[4:6] + "." + string_value[6:8]
            print(mcs_data_format)

            url = "http://api.mediatek.com/mcs/v2/devices/" +  DEVICE_INFO['device_id'] + "/datapoints"
            headers = {"Content-type": "application/json", "deviceKey": DEVICE_INFO['device_key']}
            post_to_mcs = requests.post(url, headers=headers, data=json.dumps(mcs_data_format))

            downlink_data[0]['macAddr'] = Target_node_MAC
            downlink_data[0]['id'] = str(int(time.time()))
            client.publish(GIOT_DLTopic_prefix + GW_ID, payload=json.dumps(downlink_data), qos=0, retain=False)
            print(downlink_data)

# Connect MQTT info
client = mqtt.Client(client_id="", protocol=mqtt.MQTTv31)
client.on_connect = on_connect
client.on_message = on_message
client.username_pw_set("", password="")
client.connect("140.127.196.98", port=1883, keepalive=60)                # MQTT IP address

# Blocking call that processes network traffic, dispatches callbacks and
# handles reconnecting.
# Other loop*() functions are available that give a threaded interface and a
# manual interface.

if __name__ == '__main__':
    channel = establishCommandChannel()
    _thread.start_new_thread(waitAndExecuteCommand, (channel,))
    client.loop_forever()
    while (True):
        pass
