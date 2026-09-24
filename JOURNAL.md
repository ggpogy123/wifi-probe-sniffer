# Journal (26 hrs)

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

---
# August 23-24 (11.5 hours)

What I did
My physical ESP32 finally arrived in the mail on thursday but i had a test on friday so i had stalled the project for a bit. As the deadline was preponed, I decided to sit down and pull an all-nighter to migrate the entire project from the Wokwi simulator onto the real deal and finish everything. This migration took quite a bit as my ide was acting up a lot and wasnt detecting my esp at first and i had to rely on ai to get everything to speed. I also did everything in phases as i was working all night and submitted only a single devlog.

### Phase 1: Real Hardware testing and Channel Hopping implementation (2 hrs)
I connected the ESP32 and tested the code with real Wi-Fi. It immediately booted and initialized promiscuous mode, but it was barely catching any probe packets at all. After asking AI why my board was missing active phones nearby, it explained that the Wi-Fi radio stays locked to Channel 1 by default, meaning I was missing roughly 90% of the broadcast traffic happening across all the other 2.4GHz channels. I then implemented a hopChannel() function that cycles through channels 1 to 13 every 500 milliseconds.

### Phase 2: Debugging Crashes, switch to FreeRTOS Architecture (3.5 hrs)
Once channel hopping was active and the ESP32 started picking up dozens of real phones nearby, the board started crashing constantly in an infinite reboot loop, giving a guru something error on the serial monitor.
I pasted the crash dumps into Claude to figure out what was breaking. It explained that the sniffer_callback() function runs directly inside the Wi-Fi interrupt. Because I was doing slow I2C display drawing (display.display()), serial prints, and dynamic String manipulation inside that callback, it was blocking the CPU for too long. The hardware watchdog thought the board had frozen and forcibly reset it.
Claude and Gemini helped me restructure the whole firmware to use FreeRTOS. I created a queue `probeQueue` and wrote the callback to only extract raw bytes into a compact struct and push it using `xQueueSendFromISR`. Then I created a dedicated background worker task `probeWorkerTask` pinned to Core 0 using `xTaskCreatePinnedToCore`. This worker task sits in a loop, listens for new packets coming out of the queue, and handles all the heavy display drawing and serial prints without blocking the Wi-Fi hardware.

### Phase 3: Persistent Flash Storage with LittleFS (2 hrs)
One big problem I noted back on Day 5 was that whenever the ESP32 loses power or restarts, all the tracked devices and logs disappear from RAM. To solve this without needing an external SD card module, I used LittleFS to treat the ESP32's built-in flash memory like a storage drive. With the help of AI to understand the LittleFS syntax, I wrote a logging function that writes each captured probe into a`/probes.csv` file with timestamps, MAC, SSID, RSSI, vendor, and MAC type. I also added a failsafe check so that if LittleFS fails to mount for any reason, the device just continues running in live memory-only mode without crashing.

### Phase 4: OUI Vendor Lookup and MAC Randomization Check (1 hrs)
I wanted the sniffer to tell me what kind of device is scanning nearby instead of just showing raw hexadecimal MAC addresses. I asked Claude to generate a list of around 80 common device manufacturer OUI prefixes (Apple, Samsung, Google, Xiaomi, OnePlus, Espressif, Intel, Motorola, etc.) and format them into a clean struct array. Claude also suggested storing this table in flash memory using the `PROGMEM` keyword so it would not eat up RAM.
I also found out that we can easily tell if a device is hiding its mac address by randomizing it. We can tell if it has been randomized by checking the first byte of the MAc address.

### Phase 5: 4-Screen UI System implementation using external button (~2.5 hrs)
With all this new data (vendors, proximity, randomized status, uptime, storage state), everything became too crowded for a single OLED display. I wired a second push button to GPIO 18 `BUTTON_NEXT` so I could cycle through different screens. This has a 4 screen setup:
 Screen 0: Live probe view.
 Screen 1: Overall statistics, total probe count, unique device count, uptime, and LittleFS status.
 Screen 2: Most suspicious device detected so far based on repeat sighting scores.
  Screen 3: Detailed view of the last captured probe including MAC type and vendor.
I also updated the reset button on GPIO 19 so that holding it clears the RAM and also wipes the `/probes.csv` file.

## What I learned
 - Why FreeRTOS tasks and queues are essential for real-time embedded system? Interrupt callbacks must be non-blocking and lightning fast, while slow tasks like I2C screen drawing and file I/O should be pushed to background threads.
- Multicore process working on the ESP32 by assigning tasks to Core 0 while the Wi-Fi stack runs on Core 1.
- How to use LittleFS to store structured CSV files directly on internal memory.
- How MAC address standards work. And how to tell if a MAC address is randomised.
- How the `PROGMEM` keyword works.

## Screenshots
<img width="2559" height="1471" alt="image" src="https://github.com/user-attachments/assets/a7eac239-f83b-4dbc-afe3-4e6d522756c3" />
<img width="606" height="335" alt="image" src="https://github.com/user-attachments/assets/7a6be0bd-988a-4125-a5df-741d0325ed8a" />
<img width="694" height="385" alt="image" src="https://github.com/user-attachments/assets/601d21f6-6cc4-4cde-88b0-cb4ac2af97ef" />
<img width="2532" height="1573" alt="image" src="https://github.com/user-attachments/assets/7c76dcb1-1f02-4593-b9b5-76ee66c6b43a" />
<img width="2559" height="1599" alt="image" src="https://github.com/user-attachments/assets/3e42d909-d70a-46f3-aca2-dd8ea04e3e2f" />

## Problems Encountered
- Memory Corruption in FreeRTOS: When I first set up the queue, I tried passing String objects inside the struct. The board kept crashing because the memory kept corrupting over the task contexts. Gemini taught me how to make a plain C struct `ProbePacket` with fixed-size byte arrays `uint8_t mac[6]` and `char ssid[33]` so memory stays safe and the board doesnt craash.
 -  OLED Screen Overflow: Trying to show SSID, MAC, vendor, RSSI, threat level, and probe counts on one small screen caused text to overlap and clip off the edges. Splitting the UI across 4 switchable screens solved this perfectly.
 -  Wi-Fi Dropped Packets: Before adding the FreeRTOS queue, testing in an area with 5 active smartphones caused the ESP32 to drop over half the probes due to I2C blocking. Making the callback separate from the display completely resolved the drops.
 -  I do not have an oled at hand to test the display but i have verified that everything used to work on wokwi and gemini also says it should work perfectly.
