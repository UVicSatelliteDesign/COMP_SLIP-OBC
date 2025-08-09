import RPi.GPIO as GPIO
import time
import serial
import os

# Pin definitions
CAM1_PD_PIN = 17
CAM1_RT_PIN = 18
CAM_SEL_PIN = 27
CAM2_PD_PIN = 22
CAM2_RT_PIN = 23
TRANSCIEVER_EXTI_PIN = 24

def setup_gpio():
    """Sets up the GPIO pins on the Raspberry Pi."""
    GPIO.setmode(GPIO.BCM)
    GPIO.setup(CAM1_PD_PIN, GPIO.OUT)
    GPIO.setup(CAM1_RT_PIN, GPIO.OUT)
    GPIO.setup(CAM_SEL_PIN, GPIO.OUT)
    GPIO.setup(CAM2_PD_PIN, GPIO.OUT)
    GPIO.setup(CAM2_RT_PIN, GPIO.OUT)
    GPIO.setup(TRANSCIEVER_EXTI_PIN, GPIO.IN)
    print("GPIO setup complete.")

def emulate_sensors():
    """Emulates sensor values by controlling GPIO pins."""
    print("Emulating sensor values...")
    # Example: Toggle camera power and select
    GPIO.output(CAM1_PD_PIN, GPIO.HIGH)
    time.sleep(1)
    GPIO.output(CAM_SEL_PIN, GPIO.HIGH)
    time.sleep(1)
    GPIO.output(CAM1_PD_PIN, GPIO.LOW)
    time.sleep(1)
    GPIO.output(CAM_SEL_PIN, GPIO.LOW)
    print("Sensor emulation finished.")

def wait_for_serial_and_read():
    """Waits for the serial port to be available, then reads data."""
    serial_port = '/dev/ttyACM0'
    print(f"Waiting for serial port {serial_port} to become available...")
    
    # Wait for the STM32 to be flashed and the serial port to appear
    while not os.path.exists(serial_port):
        time.sleep(1)
    
    print(f"Serial port {serial_port} is available.")
    time.sleep(2)  # Give the serial port some time to initialize

    # Now that the board is ready, emulate the sensors
    emulate_sensors()

    try:
        ser = serial.Serial(serial_port, 115200, timeout=10)
        print(f"Reading from serial port {serial_port}...")
        while True:
            line = ser.readline().decode('utf-8').strip()
            if line:
                print(f"Received: {line}")
            else:
                print("No data received for 10 seconds. Exiting.")
                break
    except serial.SerialException as e:
        print(f"Error reading from serial port: {e}")
    finally:
        if 'ser' in locals() and ser.is_open:
            ser.close()

def main():
    """Main function to run the hardware-in-the-loop test."""
    try:
        setup_gpio()
        wait_for_serial_and_read()
    finally:
        GPIO.cleanup()
        print("GPIO cleaned up.")

if __name__ == "__main__":
    main() 