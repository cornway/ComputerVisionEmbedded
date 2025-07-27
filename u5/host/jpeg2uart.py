import cv2
import serial
import time

# --- Configuration ---
VIDEO_DEVICE = 0               # change if you have multiple cameras
SERIAL_PORT  = '/dev/ttyUSB0'  # replace with your serial device
BAUDRATE     = 115200          # match on the receiving end
WIDTH, HEIGHT = 80, 80       # target resolution

# --- Initialize camera and serial ---
cap = cv2.VideoCapture(VIDEO_DEVICE)
if not cap.isOpened():
    raise RuntimeError(f"Cannot open camera #{VIDEO_DEVICE}")

ser = serial.Serial(SERIAL_PORT, BAUDRATE, timeout=1)
time.sleep(2)  # give some time for the serial port to initialize

print(f"Streaming from camera #{VIDEO_DEVICE} at {WIDTH}x{HEIGHT}.")
print(f"Press 'c' to capture & send JPEG over {SERIAL_PORT}.")

try:
    while True:
        # 1. Capture frame
        ret, frame = cap.read()
        if not ret:
            print("Failed to grab frame")
            break

        # 2. Resize to 160x120
        frame_small = cv2.resize(frame, (WIDTH, HEIGHT))

        # 3. Compress to JPEG in memory
        encode_param = [int(cv2.IMWRITE_JPEG_QUALITY), 90]  # quality 0-100
        success, jpeg_buf = cv2.imencode('.jpg', frame_small, encode_param)
        if not success:
            print("JPEG encoding failed")
            continue

        # 4. Show resulting frame
        cv2.imshow('Resized JPEG Preview', frame_small)

        req = 0
        rd_data = ser.read(1)
        if (len(rd_data)):
            req = int.from_bytes(rd_data)

        #assert req == 0xff, f'req {req} != 0xff'
        
        # 5. Wait for key press
        if (req == 0xff):
            key = ord('c')
        else:
            key = cv2.waitKey(1) & 0xFF
        if key == ord('c'):
            # 6. Send raw JPEG bytes to serial
            data = jpeg_buf.tobytes()
            length = len(data)

            if length > 0xFFFF:
                print("Frame too large to prefix with 2 bytes length!")
                continue

            length_prefix = length.to_bytes(2, byteorder='little')
            packet = length_prefix + data
            ser.write(packet)
            ser.flush()
            print(length_prefix[0], length_prefix[1])
            print(f"Sent {len(data)} bytes")

        elif key == ord('q'):
            # gracefully quit
            print("Quitting.")
            break

finally:
    # clean up
    cap.release()
    ser.close()
    cv2.destroyAllWindows()
