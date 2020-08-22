The application calls the specified script when receiving a message in the topic, the result of the script execution allows you to send messages to the specified topics. To send messages, it is necessary as a result of the script to return lines in the format `<topic name> <message>`, each processed line must end with a line feed character.

```
  -h MQTT server.
	-p MQTT server port.
	-u Username.
	-k Password.
	-t Topic for subscription.
	-s Triggered script.
	-i Init script.
```
