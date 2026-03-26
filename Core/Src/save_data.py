import serial
import time

# CONFIGURATION
SERIAL_PORT = 'COM10'  # <--- CHECK DEVICE MANAGER FOR YOUR PORT!
BAUD_RATE = 115200    # Virtual COM ports usually ignore this, but 115200 is standard
FILENAME = 'frequency_data.csv'

def main():
    try:
        # Open the connection
        ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
        print(f"Listening on {SERIAL_PORT}... (Press Ctrl+C to stop)")
        print(f"Data will be appended to: {FILENAME}")

        # Open file in APPEND mode ('a') so we don't overwrite previous saves
        with open(FILENAME, 'a') as f:
            while True:
                # 1. Check if data is waiting in the USB buffer
                if ser.in_waiting > 0:
                    try:
                        # 2. Read the line
                        line = ser.readline().decode('utf-8', errors='ignore').strip()
                        
                        # 3. Print to console (so you know it's working)
                        print(line)
                        
                        # 4. Save to file
                        f.write(line + '\n')
                        
                    except Exception as e:
                        print(f"Error reading line: {e}")
                
                # Sleep tiny amount to save CPU usage on PC
                else:
                    time.sleep(0.01)

    except serial.SerialException:
        print(f"ERROR: Could not open {SERIAL_PORT}. Is VS Code using it?")
    except KeyboardInterrupt:
        print("\nStopping...")
        if 'ser' in locals() and ser.is_open:
            ser.close()

if __name__ == "__main__":
    main()