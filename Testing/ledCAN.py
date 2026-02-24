import can
bus = can.Bus(interface='pcan', channel='PCAN_USBBUS1', bitrate=500000)
msg = can.Message(arbitration_id=0x507, data=[0,0,0,0,0,0,0x18,0xFC], is_extended_id=False)
bus.send(msg)
