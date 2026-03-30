import socket
import numpy as np
import cv2
import struct
import time
from typing import Dict, Optional, Tuple

# ------------------------------
# Configuration
# ------------------------------
UDP_LISTEN_IP = '0.0.0.0'
UDP_LISTEN_PORT = 5100
IMAGE_WIDTH = 32
IMAGE_HEIGHT = 24
IMAGE_SIZE = IMAGE_WIDTH * IMAGE_HEIGHT
DISPLAY_WIDTH = 320  # Resized width for display
DISPLAY_HEIGHT = 240 # Resized height for display
REASSEMBLY_TIMEOUT_SEC = 1.0

FRAGMENT_HEADER = b'\xFE\x02\xFE\x02'
BINARY_HEADER = b'\xFE\x01\xFE\x01'
FRAGMENT_COUNT = 4
FRAGMENT_META_SIZE = 10


def parse_fragment(datagram: bytes) -> Tuple[Optional[dict], Optional[str]]:
    if len(datagram) < FRAGMENT_META_SIZE:
        return None, f"short fragment ({len(datagram)} bytes)"

    if datagram[0:4] != FRAGMENT_HEADER:
        return None, "invalid fragment header"

    sequence = struct.unpack_from('<H', datagram, 4)[0]
    fragment_index = datagram[6]
    fragment_count = datagram[7]
    payload_size = struct.unpack_from('<H', datagram, 8)[0]
    payload = datagram[10:]

    if fragment_count != FRAGMENT_COUNT:
        return None, f"unexpected fragment count {fragment_count}"

    if fragment_index >= fragment_count:
        return None, f"invalid fragment index {fragment_index}"

    if len(payload) != payload_size:
        return None, f"fragment payload mismatch: got {len(payload)} expected {payload_size}"

    return {
        'sequence': sequence,
        'fragment_index': fragment_index,
        'payload': payload,
    }, None


def parse_combined_packet(packet: bytes) -> Tuple[Optional[dict], Optional[str]]:
    if len(packet) < 6:
        return None, f"packet too short after reassembly ({len(packet)} bytes)"

    if packet[0:4] != BINARY_HEADER:
        return None, "invalid binary packet header"

    packet_size = int.from_bytes(packet[4:6], byteorder='little')
    packet_data = packet[6:]

    if len(packet_data) != packet_size:
        return None, f"binary payload {len(packet_data)} bytes, expected {packet_size}"

    expected_image_bytes = IMAGE_SIZE * 4
    if packet_size < expected_image_bytes + 1:
        return None, f"binary payload too short for image data ({packet_size} bytes)"

    image_bytes = packet_data[0:768]
    step1_bytes = packet_data[768:1536]
    step2_bytes = packet_data[1536:2304]
    step3_bytes = packet_data[2304:3072]

    num_detected = packet_data[3072]
    person_data_start = 3073

    detected_people = []
    for i in range(num_detected):
        if person_data_start + 4 > len(packet_data):
            return None, f"incomplete person data for person {i}"

        y, x, area = struct.unpack('<BBH', packet_data[person_data_start:person_data_start + 4])
        detected_people.append({'x': x, 'y': y, 'area': area})
        person_data_start += 4

    if person_data_start != len(packet_data):
        return None, f"unexpected trailing bytes after detections ({len(packet_data) - person_data_start})"

    return {
        'image_bytes': image_bytes,
        'step1_bytes': step1_bytes,
        'step2_bytes': step2_bytes,
        'step3_bytes': step3_bytes,
        'detected_people': detected_people,
        'num_detected': num_detected,
    }, None


def cleanup_expired_frames(pending_frames: Dict[Tuple[str, int], dict], now: float) -> None:
    expired_keys = [
        key for key, state in pending_frames.items()
        if now - state['created_at'] > REASSEMBLY_TIMEOUT_SEC
    ]
    for key in expired_keys:
        del pending_frames[key]

# ------------------------------
# Main Program
# ------------------------------
def main():
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock.bind((UDP_LISTEN_IP, UDP_LISTEN_PORT))
        sock.settimeout(1.0)
        print(f"Listening on UDP {UDP_LISTEN_IP}:{UDP_LISTEN_PORT}")
    except OSError as e:
        print(f"Error opening UDP socket: {e}")
        return

    # Calculate scaling factors for drawing on the resized image
    scale_x = DISPLAY_WIDTH / IMAGE_WIDTH
    scale_y = DISPLAY_HEIGHT / IMAGE_HEIGHT

    # blank image now twice width & height
    blank_image = np.zeros((DISPLAY_HEIGHT * 2, DISPLAY_WIDTH * 2, 3), dtype=np.uint8)
    cv2.putText(blank_image, "Waiting for data...", (DISPLAY_WIDTH // 2, DISPLAY_HEIGHT),
                cv2.FONT_HERSHEY_SIMPLEX, 1, (255, 255, 255), 2)
    cv2.imshow("Thermal Detection", blank_image)

    pending_frames: Dict[Tuple[str, int], dict] = {}

    while True:
        try:
            datagram, sender = sock.recvfrom(4096)
            cleanup_expired_frames(pending_frames, time.time())

            fragment, err = parse_fragment(datagram)
            if err:
                print(f"Warning: {err} from {sender[0]}:{sender[1]}")
                continue
            if fragment is None:
                continue

            frame_key = (sender[0], fragment['sequence'])
            state = pending_frames.setdefault(frame_key, {
                'created_at': time.time(),
                'fragments': {},
            })
            state['fragments'][fragment['fragment_index']] = fragment['payload']

            if len(state['fragments']) < FRAGMENT_COUNT:
                continue

            packet = b''.join(state['fragments'][index] for index in range(FRAGMENT_COUNT))
            del pending_frames[frame_key]

            parsed, err = parse_combined_packet(packet)
            if err:
                print(f"Warning: {err} from {sender[0]}:{sender[1]}")
                continue
            if parsed is None:
                continue

            image_bytes = parsed['image_bytes']
            step1_bytes = parsed['step1_bytes']
            step2_bytes = parsed['step2_bytes']
            step3_bytes = parsed['step3_bytes']
            detected_people = parsed['detected_people']

            # convert to images
            def to_display(img_bytes):
                arr = np.frombuffer(img_bytes, dtype=np.uint8).reshape((IMAGE_HEIGHT, IMAGE_WIDTH))
                arr_flip = np.flip(arr, axis=1)  # flip horizontally
                arr_float = arr_flip.astype(np.float32)
                min_val = float(arr_float.min())
                max_val = float(arr_float.max())
                if max_val > min_val:
                    norm = ((arr_float - min_val) * (255.0 / (max_val - min_val))).astype(np.uint8)
                else:
                    norm = np.zeros_like(arr_flip, dtype=np.uint8)
                col = cv2.applyColorMap(norm, cv2.COLORMAP_INFERNO)
                return cv2.resize(col, (DISPLAY_WIDTH, DISPLAY_HEIGHT), interpolation=cv2.INTER_NEAREST)
            
            def get_label_mask(step3_bytes, label):
                arr = np.frombuffer(step3_bytes, dtype=np.uint8).reshape((IMAGE_HEIGHT, IMAGE_WIDTH))
                return (arr == label).astype(np.uint8) * 255

            display_orig  = to_display(image_bytes)
            display_step1 = to_display(step1_bytes)
            display_step2 = to_display(step2_bytes)
            display_step3 = to_display(step3_bytes)

            cv2.putText(display_orig, "Raw Image", (10, 20),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 255, 255), 1)
            cv2.putText(display_step1, "Background", (10, 20),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 255, 255), 1)
            cv2.putText(display_step2, "Distance Map", (10, 20),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 255, 255), 1)
            cv2.putText(display_step3, "Watershed", (10, 20),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 255, 255), 1)

            # draw detections on original only
            scale_x = DISPLAY_WIDTH / IMAGE_WIDTH
            scale_y = DISPLAY_HEIGHT / IMAGE_HEIGHT

            for idx, person in enumerate(detected_people):
                x_orig, y_orig, area = person['x'], person['y'], person['area']

                # Extract blob mask from step3
                label = idx + 1  # assuming labels are 1..N
                mask_small = get_label_mask(step3_bytes, label)

                # Resize mask to display size
                mask_large = cv2.resize(mask_small, (DISPLAY_WIDTH, DISPLAY_HEIGHT), interpolation=cv2.INTER_NEAREST)

                # Create colored overlay
                overlay = display_orig.copy()
                overlay[mask_large > 0] = (0, 255, 0)  # bright green blob

                # Blend overlay
                display_orig = cv2.addWeighted(display_orig, 0.7, overlay, 0.3, 0)

                # Improved readable text
                text_pos = (int(x_orig * scale_x), int(y_orig * scale_y))

                # Black outline
                cv2.putText(display_orig, f"({x_orig},{y_orig})", text_pos,
                            cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 0, 0), 3)
                cv2.putText(display_orig, f"({x_orig},{y_orig})", text_pos,
                            cv2.FONT_HERSHEY_SIMPLEX, 0.6, (255, 255, 0), 1)

                cv2.putText(display_orig, f"Area:{area}", (text_pos[0], text_pos[1] + 20),
                            cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 0, 0), 3)
                cv2.putText(display_orig, f"Area:{area}", (text_pos[0], text_pos[1] + 20),
                            cv2.FONT_HERSHEY_SIMPLEX, 0.6, (255, 255, 0), 1)

            # compose 2×2 grid
            top = np.hstack((display_orig, display_step1))
            bottom = np.hstack((display_step2, display_step3))
            combined = np.vstack((top, bottom))

            cv2.imshow("Thermal Detection", combined)

            if cv2.waitKey(1) & 0xFF == ord('q'):
                break

        except socket.timeout:
            cleanup_expired_frames(pending_frames, time.time())
            cv2.imshow("Thermal Detection", blank_image)
            if cv2.waitKey(1) & 0xFF == ord('q'):
                break
        except Exception as e:
            print(f"An unexpected error occurred: {e}. Attempting to resync...")
            time.sleep(0.1)
            cv2.imshow("Thermal Detection", blank_image)
            if cv2.waitKey(1) & 0xFF == ord('q'):
                break

    # Clean up
    sock.close()
    cv2.destroyAllWindows()
    print("UDP socket closed and OpenCV windows destroyed.")

if __name__ == "__main__":
    main()