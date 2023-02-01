# esphome

WR3223 controller:

Files not needed:
- energy-meter.yaml 
- hochbeet-conroller.yaml


Features:

- error flags decoding
- 
- store or restore default config from 'EPROM'
- auto detect of control panel -> if not attached, controller will write current state
- status update at least every 20 seconds (default: 10s, can be configured in the UI)

Connection:
![Connection]((https://github.com/schmurgel-tg/esphome/blob/main/20230101_174032.jpg?raw=true))

THX to:
https://github.com/frami/org.openhab.binding.wr3223
