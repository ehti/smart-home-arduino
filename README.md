
#Smart Home
##Manage electrical appliances remotely via SMS and Internet


It's a home automation project that I coded for fun and utility, which can be used to monitor and manage electrical appliances connected to the system remotely in two ways:
- SMS/Text message codes
- Web-based control panel (requires internet)

In its current form, the system has two outputs. You can connect them to anything, but to actually control AC appliances like lights, fans or air conditioners, **at least one should be connected to a power strip/board using relays**. Arduino pin outputs can't directly be connected to 110v/220v appliances! :)

Following are the required hardware components:
- Arduino board (Uno, Mega or other compatible boards)
- GSM modem (e.g. SIM900D shield)
- Ethernet Shield
- Relays 

Pin 8 and 9 are the outputs that can be remotely monitored and controlled, which you can obviously modify in the code. Similarly, pattern of text messages for control can be modified.

The code is crude, but does the job. Fork it or send over pull requests for any imporvements, and obviously you can freely reuse the code. ;-) 
