# Journal (14.5 hrs)

## July 28 - Day 1 (2 hrs)

### What I did
Decided on the project, set up the GitHub repo and Wokwi ESP32 project, read some documentation and about wifi probe requests 
and wrote the project description.

### Why I picked this project
I was planning on choosing between a network traffic analyzer, a GPS tracker and a 
WiFi probe sniffer. The probe sniffer felt the most spy like
because it requires no network connection at all, it's completely passive 
and the target has no idea it's happening.

### What I learned
Didn't realize phones constantly broadcast the SSIDs of every network they've ever connected to, 
I thought this only happened when you turned on WiFi or refreshed it from the settings app or maybe using geolocation.
Turns out it happens every few seconds automatically.

Read through these to understand how it works:
- [ESP32 WiFi API](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/wifi.html)
- [ESP32 Promiscuous Mode](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/network/esp_wifi.html)
  
- [WiFi Probe Requests — Wikipedia](https://en.wikipedia.org/wiki/Probe_request)

### Screenshots
<img width="2559" height="1473" alt="image" src="https://github.com/user-attachments/assets/2e62fd50-68b4-47f6-8d01-58eacb8c7051" />
<img width="2559" height="1445" alt="image" src="https://github.com/user-attachments/assets/793581fd-4695-4784-a19c-8e0b44f90c0a" />





### Problems
Nothing till now as i'm also using AI to make me understand wherever i get stuck.


---

## July 29 - Day 2 (1.5 hrs)

### What I did
I set up the environment on Wokwi and initialized the WiFi adapter in promiscuous mode using `esp_wifi` APIs. I also verified serial output to make sure the board boots and enters promiscuous mode nicely.


### What I learned
Standard WiFi connection modes require joining an Access Point, but promiscuous mode allows the ESP32 radio antenna to passively capture raw 802.11 packets floating around on the set channel without authenticating to any network.

### Documentation followed:
- [ESP-IDF WiFi Promiscuous Mode](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/network/esp_wifi.html#promiscuous-mode)

### Screenshots
<img width="2559" height="1477" alt="image" src="https://github.com/user-attachments/assets/8959bab3-68d7-49f9-bb8b-1f33b94c532a" />


### Problems
Wokwi can't simulate this Wi-Fi thing (no actual  devices broadcasting packets in the simulation), so real packets won't trigger automatically in the simulator. Will need to simulate incoming packet data for the same later.

---

## August 1 and 2 - Day 2-3 (6 hrs)
### What i did
Couldnt update the journal yesterday as some work came up when i started to write so i am combining both days into one.
So basically what i did was attach an oled display to the esp so it displays the wifi ssid,mac addresses and the RSSI's on the go and i do not have to connect a pc to the esp to see the output.
I then made it to show live probe data instead of just printing to the serial monitor like before. (I'm still working with fake probe data, i am trying to get my hands on a real esp to test everything).
Everytime a probe comes in, the screen updates and shows the probe count, the SSID, the MAC address, and how strong the signal was (in dBm).
Btw i connected the SDA and SCL to pin 22 and 21 of the esp respectively.

### What I learned
For printing stuff on the screen I used the adafruit SSD1306 library. I thought it would be very complicated and hard at first as i haven't worked with an oled display before but it's actually pretty easy,it handles all the stuff internally and i just had to tell it where to put the cursor and call .println() exactly like Serial. I made an updateDisp() function that clears the screen and makes everything from scratch each time a new probe comes in.

One more thing i learned is that there exists something called hidden ssids. To connect to these you have to enter the ssid name and the password as well. So i made it so that it prints a `(hidden)` ssid when the ssid is in the hidden state. This makes it look much better when encountering a hidden ssid.

Instead of just printing one line to serial, Claude suggested to make a FakeProbe struct that holds the MAC, SSID, and RSSI together. I did it and it makes things more organized than hardcoding the strings.

Most of the code writing has been done by me, I used Claude to understand the docs wherever I got stuck and wherever I couldnt fix the code by myself.

### Screenshots
<img width="2559" height="1412" alt="image" src="https://github.com/user-attachments/assets/3abbacaf-146e-4294-9f9e-a5dc0f5652c3" />
<img width="2559" height="1408" alt="image" src="https://github.com/user-attachments/assets/434ed8b4-f8e4-4ec2-911a-9eb11f60a680" />
<img width="2559" height="1336" alt="image" src="https://github.com/user-attachments/assets/846ce8ff-dc97-4242-be37-2771d6cf8b32" />

---

## August 5- Day 4 (1.5 hrs)
### What i did
Today I implemented a button to clear the screen and reset the probe logs. I wired one side of the button to pin `19` and the other directly to `GND`. I also restructured the main loop of the code with the help of AI to handle the button input properly.
### What I learned
Because Wokwi can't simulate the real Wi-Fi packets, we had hardcoded the probes and I was using `delay(3000)` to have gap between them. It turns out that `delay()` freezes the ESP32 completely for 3 seconds and the board becomes completely unresponsive.

To fix this, I learned how to use `millis()` to track time instead. By adding a variable (`lastProbeTime`) and checking if 3000 milliseconds have passed, the loop() can keep running many times a second. This means it can constantly listen for the button press and react instantly while still waiting the full 3 seconds before showing the next fake probe.

I also figured out how the buttons actually work in the simulator. You have to wire them diagonally to guarantee the circuit only closes when you physically click it down. I earlier connected in the same direction and it was keeping it pressed down which was annoying to say at the least lol.
### Screenshots
<img width="2559" height="1471" alt="image" src="https://github.com/user-attachments/assets/e1010348-88d5-489f-9729-74d4b475d083" />
<img width="673" height="93" alt="image" src="https://github.com/user-attachments/assets/6530fdf8-be8f-4c44-b15c-66a20a730957" />
<img width="703" height="563" alt="image" src="https://github.com/user-attachments/assets/540b31d2-3ddf-44d2-ac00-7678a8c2290c" />

### Problems Encountered
My button was completely unresponsive at first and then when I was messing randomly with the code, it started automatically clearing the log in an infinite loop without me even pressing it. I used AI to debug the issue, which pointed out that the delay function was blocking the button reads and helped me write the `millis()` thing to fix it permanently.

---
## August 16- Day 5 (3.5 hrs)
## What i did
Today I added the suspicious person detection logic to the sniffer. The ESP32 now tracks every unique MAC address it sees and flags it as suspicious if it disappears and comes back multiple times over a long period of time. I also swapped out the fake probe simulation entirely and wrote the function that would work on a physical ESP32 picking up actual wifi packets.
## What i learned
My first idea for flagging suspicious devices was to count how many times a MAC appears in total, but that immediately falls apart as a phone sitting next to the device for an hour would just keep pinging and rack up a huge count even though it never left.

Then I thought about using a time window, like flagging if a MAC appears more than 3 times within 60 seconds. But that has the opposite problem, a phone pings every 20-60 seconds naturally, so within 60 seconds you'd only ever see it once or twice. It would never trigger.

The approach that actually makes sense is tracking first seen and last seen timestamps per MAC. A device is only flagged if it has been seen 3 or more times AND the gap between its first and last sighting is over 30 minutes. That way a phone sitting nearby all day doesn't get flagged, but a device that keeps disappearing and reappearing near you across a long period does.

I also learned how the real promiscuous mode callback works. Instead of us calling a function myself, i registered a sniffer_callback with the hardware using esp_wifi_set_promiscuous_rx_cb() and the ESP32 calls it automatically every time the antenna picks up a frame. Inside the callback, probe requests have the frame type 0x40 in the first byte, the sender MAC is always at bytes 10-15, and the SSID length and text are at bytes 25 and so on. So I  just read from those fixed positions directly.

## Screenshots

<img width="1049" height="1044" alt="image" src="https://github.com/user-attachments/assets/4a754320-1285-4ddc-bd78-bc7583e4adba" />
<img width="928" height="308" alt="image" src="https://github.com/user-attachments/assets/814d7cb6-7ffe-47c9-a440-6186c20e7ca5" />

## Problems Encountered
I was so busy in my exams that i couldnt work on this for around 2 weeks so i essentially forgot about what all i had done and had to check everything again lol. I have ordered an esp to check this code and it will arrive soon. This is why i ported the entire code to work on real hardware. One issue is that if the esp loses power, the whole cache is lost in which it stores the list of mac addessres for the suspicion detection part.


