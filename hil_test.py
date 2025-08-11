import os
import sys
import time
import serial

try:
    import pigpio
except ImportError:
    pigpio = None


class AltimeterMS5611Emulator:
    """
    Minimal MS5611-compatible emulator for address 0x76.
    Matches STM32 transactions in `project/Core/Src/obc_interface.c`:
      - Reset:           0x1E
      - Convert D1:      0x48 (pressure)
      - Convert D2:      0x58 (temperature)
      - ADC read:        0x00 (then master reads 3 bytes)
      - PROM reads:      0xA2, 0xA4, 0xA6, 0xA8, 0xAA, 0xAC (each 2 bytes)
    """

    def __init__(self) -> None:
        self._last_conv = None  # 'D1' or 'D2'
        # Fixed PROM coefficients (C1..C6). Values are arbitrary but consistent.
        self._prom = {
            0xA2: (0x6A, 0x86),  # C1
            0xA4: (0x67, 0x12),  # C2
            0xA6: (0x7B, 0x45),  # C3
            0xA8: (0x5C, 0xE3),  # C4
            0xAA: (0x83, 0x19),  # C5
            0xAC: (0x73, 0xB2),  # C6
        }
        # Synthetic base values for pressure/temperature ADC (24-bit)
        self._d1_base = 0x640000  # pressure
        self._d2_base = 0x500000  # temperature
        self._tick = 0

    def handle_write(self, payload: bytes) -> bytes | None:
        # Process command bytes from master (write transfers).
        tx = None
        for byte in payload:
            if byte == 0x1E:  # Reset
                self._last_conv = None
                self._tick = 0
            elif byte == 0x48:  # D1 conversion (pressure)
                self._last_conv = 'D1'
                self._tick += 1
            elif byte == 0x58:  # D2 conversion (temperature)
                self._last_conv = 'D2'
                self._tick += 1
            elif byte == 0x00:  # ADC read request will follow as a read
                # Prepare 3 output bytes immediately
                if self._last_conv == 'D1':
                    value = (self._d1_base + (self._tick & 0xFF)) & 0xFFFFFF
                elif self._last_conv == 'D2':
                    value = (self._d2_base + (self._tick & 0xFF)) & 0xFFFFFF
                else:
                    value = 0x000000
                tx = bytes([(value >> 16) & 0xFF, (value >> 8) & 0xFF, value & 0xFF])
            elif byte in self._prom:  # PROM read command
                # Master will do a 2-byte read next
                hi, lo = self._prom[byte]
                tx = bytes([hi, lo])
            else:
                # Ignore unknown commands
                pass
        return tx


class Temp12BitEmulator:
    """
    Minimal 12-bit temperature sensor emulator at given address, responding to
    an 8-bit register 0x00 read of 2 bytes. The STM32 packs as:
      temp_raw = (raw[0] << 4) | (raw[1] >> 4)
    """

    def __init__(self, raw_value_12bit: int = 0x640) -> None:
        self._raw12 = max(0, min(0xFFF, raw_value_12bit))
        self._reg_ptr = 0x00

    def handle_write(self, payload: bytes) -> bytes | None:
        tx = None
        for i, byte in enumerate(payload):
            if i == 0:
                # Register pointer
                self._reg_ptr = byte
                if self._reg_ptr == 0x00:
                    hi = (self._raw12 >> 4) & 0xFF
                    lo = (self._raw12 & 0x0F) << 4
                    tx = bytes([hi, lo])
            else:
                # Ignore writes to config registers for now
                pass
        return tx


class I2CSlave:
    """
    Thin wrapper over pigpio BSC to expose an I2C slave at a given address and
    serve data prepared by an emulator.
    Uses GPIO 18 (SDA) and GPIO 19 (SCL) as per BSC peripheral.
    """

    def __init__(self, address: int, emulator: object) -> None:
        if pigpio is None:
            raise RuntimeError("pigpio is required for I2C slave emulation. Install pigpio and run pigpiod.")
        self._address = address & 0x7F
        self._emulator = emulator
        self._pi = pigpio.pi()
        if not self._pi.connected:
            raise RuntimeError("Failed to connect to pigpio daemon. Ensure pigpiod is running.")
        # Enable BSC peripheral in I2C slave mode
        self._pi.bsc_i2c(self._address)

    def close(self) -> None:
        try:
            self._pi.bsc_i2c(0)  # disable
        finally:
            self._pi.stop()

    def serve_forever(self, run_seconds: float | None = None) -> None:
        start = time.time()
        pending_tx = b""
        try:
            while True:
                now = time.time()
                if run_seconds is not None and (now - start) > run_seconds:
                    break
                # Provide last prepared TX (if any). This call also collects RX.
                status, rx_bytes = self._xfer(pending_tx)
                pending_tx = b""  # consumed

                if status < 0:
                    # Brief sleep to avoid tight loop on errors
                    time.sleep(0.001)
                    continue

                if rx_bytes:
                    prepared = self._emulator.handle_write(rx_bytes)
                    if prepared:
                        # Queue bytes to be clocked out on the master's next read
                        pending_tx = prepared
                else:
                    # Idle
                    time.sleep(0.0005)
        finally:
            pass

    def _xfer(self, tx: bytes) -> tuple[int, bytes]:
        # pigpio bsc_xfer takes a dict with 'txBuf'
        x = self._pi.bsc_xfer(tx)
        status = x.get('status', 0)
        rx_len = x.get('rxLen', 0)
        rx = x.get('rxBuf', b"") if rx_len else b""
        return status, rx


class Vector3Emulator:
    """
    Minimal vector-3 emulator (e.g., accelerometer/gyroscope) that returns 6 bytes
    (three 16-bit big-endian signed values) when register pointer is set to 0x00.
    """

    def __init__(self, base_xyz: tuple[int, int, int] = (100, 200, -150)) -> None:
        self._reg_ptr = 0x00
        self._x, self._y, self._z = base_xyz
        self._tick = 0

    def handle_write(self, payload: bytes) -> bytes | None:
        tx = None
        for i, byte in enumerate(payload):
            if i == 0:
                self._reg_ptr = byte
                if self._reg_ptr == 0x00:
                    # Slowly vary values each call
                    self._tick = (self._tick + 1) & 0xFFFF
                    x = (self._x + (self._tick % 5))
                    y = (self._y - (self._tick % 5))
                    z = (self._z + ((self._tick // 2) % 5))
                    def pack16(n: int) -> tuple[int, int]:
                        n &= 0xFFFF
                        return (n >> 8) & 0xFF, n & 0xFF
                    xh, xl = pack16(x & 0xFFFF)
                    yh, yl = pack16(y & 0xFFFF)
                    zh, zl = pack16(z & 0xFFFF)
                    tx = bytes([xh, xl, yh, yl, zh, zl])
        return tx


def wait_for_serial_and_read(serial_port: str = '/dev/ttyACM0') -> None:
    print(f"Waiting for serial port {serial_port} to become available...")
    while not os.path.exists(serial_port):
        time.sleep(1)
    print(f"Serial port {serial_port} is available.")
    time.sleep(1.5)

    try:
        ser = serial.Serial(serial_port, 115200, timeout=5)
        print(f"Reading from serial port {serial_port}...")
        while True:
            line = ser.readline().decode('utf-8', errors='ignore').strip()
            if line:
                print(f"[STM32] {line}")
    except serial.SerialException as e:
        print(f"Serial error: {e}")
    finally:
        try:
            if 'ser' in locals() and ser.is_open:
                ser.close()
        except Exception:
            pass


def main() -> None:
    # Mode selection via env or args
    mode = os.environ.get('HIL_MODE') or (sys.argv[1] if len(sys.argv) > 1 else 'altimeter')
    if mode not in ('altimeter', 'temp', 'gyro', 'accel'):
        print("Usage: python3 hil_test.py [altimeter|temp|gyro|accel]")
        sys.exit(1)

    # Address selection
    addr_env = os.environ.get('HIL_ADDR')
    default_addr = 0x76 if mode == 'altimeter' else (0x4A if mode == 'temp' else (0x68 if mode == 'gyro' else 0x1E))
    address = int(addr_env, 0) if addr_env else default_addr

    # Optional: how long to run the I2C slave (seconds). None = forever
    run_seconds_env = os.environ.get('HIL_DURATION_S')
    run_seconds = float(run_seconds_env) if run_seconds_env else None

    # Start emulator
    if mode == 'altimeter':
        emulator = AltimeterMS5611Emulator()
    elif mode == 'temp':
        emulator = Temp12BitEmulator()
    else:
        # gyro/accel vector emulators
        emulator = Vector3Emulator()

    slave = None
    try:
        print(f"Starting I2C slave: mode={mode}, address=0x{address:02X}")
        slave = I2CSlave(address=address, emulator=emulator)

        # Run sensor emulator and serial reader concurrently (very simple coop)
        # Start serial reader in a background-like fashion using a lightweight loop
        # that interleaves with the I2C serving loop by non-blocking checks.
        # For simplicity, spawn a minimal thread-less interleave.

        last_serial_check = 0.0
        serial_started = False
        ser = None

        start = time.time()
        pending_tx = b""
        while True:
            # Serve I2C for a short slice
            status, rx_bytes = slave._xfer(pending_tx)
            pending_tx = b""
            if rx_bytes:
                prepared = emulator.handle_write(rx_bytes)
                if prepared:
                    pending_tx = prepared

            # Periodically try to start serial and read a line
            now = time.time()
            if not serial_started and (now - last_serial_check) > 0.5:
                last_serial_check = now
                port = os.environ.get('HIL_SERIAL', '/dev/ttyACM0')
                if os.path.exists(port):
                    try:
                        ser = serial.Serial(port, 115200, timeout=0)
                        serial_started = True
                        print(f"Serial attached: {port}")
                    except Exception:
                        pass
            if serial_started and ser is not None:
                try:
                    line = ser.readline().decode('utf-8', errors='ignore').strip()
                    if line:
                        print(f"[STM32] {line}")
                except Exception:
                    pass

            if run_seconds is not None and (now - start) > run_seconds:
                break

            # Tiny sleep to reduce CPU usage
            time.sleep(0.0005)

        if ser is not None:
            try:
                ser.close()
            except Exception:
                pass

    finally:
        if slave is not None:
            slave.close()


if __name__ == "__main__":
    main()