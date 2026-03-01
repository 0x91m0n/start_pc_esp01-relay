# What is it?
A reliable code that allows you to turn on your computer from anywhere in the world using blynk. It is possible to integrate with the voice assistant using web hooks.
  
# How it work?
Depending on the command, the relay receives a request to turn on power and automatically closes for 1.2 seconds or 5 seconds.
The webhook should look like this: https://fra1.blynk.cloud/external/api/update?token=YOUR_API_TOKEN&V42=1

# Setup and installation
1. First, on the blink.cloud website, you need to create a device, then get an api token and find the data streams item and create a v1pin, data type:integer. Add an Image Button element to interact with the relay.
2. Edit the code by entering your API key. Install libraries in the Arduino IDE. Flash it
3. When you turn on the relay, an access point will appear that you need to connect to for configuration.
4. Connect the board to the relay
![0x91m0n wirings](./img.jpg)
4. To test, visit the website http://esp-01.local (Your device and relay must be on the same Wi-Fi network.)
 
