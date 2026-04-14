import serial

ser = serial.Serial('/dev/cu.usbserial-0001', 115200, timeout = 1) # for Window uses COM(port) for Linux uses /dev/ttyUSB0

try:
    while True:
        if ser.in_waiting>0:
            data = ser.readline().decode('utf-8').rstrip()
            
            if "[UL] millis=" in data:
                target_hex = data.split("millis=")[1].strip()
                print(f"sent millis: {target_hex}")

            elif "[DL] Unix time from server:" in data:
                target_time = data.split("from server:")[1].split("s since")[0].strip()
                print(f"received time: {target_time}")

except KeyboardInterrupt:
    ser.close()
