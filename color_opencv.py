import cv2
import pandas as pd
import numpy as np
import time

def detect_colors(csv_data):
    # Set the desired camera resolution
    cam_width = 320
    cam_height = 320

    cap = cv2.VideoCapture(0)  # Use 0 for the default webcam
    
    # Set the camera resolution
    cap.set(cv2.CAP_PROP_FRAME_WIDTH, cam_width)
    cap.set(cv2.CAP_PROP_FRAME_HEIGHT, cam_height)

    display_text_until = 0  # Timestamp until which the text should be displayed
    text_data = {"rgb_text": "", "color_name": "", "tone": "", "x": 0, "y": 0}

    def mouse_callback(event, x, y, flags, param):
        nonlocal display_text_until, text_data
        if event == cv2.EVENT_LBUTTONDOWN:
            # Get RGB values at the mouse pointer position
            rgb_values = frame[y, x]
            print(f"RGB values at ({x}, {y}): {rgb_values}")

            # Check if the color is a red tone or green tone
            r, g, b = rgb_values[2], rgb_values[1], rgb_values[0]
            
            if 128 <= r <= 255 and g <= 200 and b <= 200 and r > g and r > b:
                tone = "Red Tone"
            elif r <= 200 and 128 <= g <= 255 and b <= 200 and g > r and g > b:
                tone = "Green Tone"
            elif 70 <= r <= 150 and 50 <= g <= 100 and b <= 75 and r > g > b:
                tone = "Red-Brownish Tone"
            elif 70 <= g <= 150 and 50 <= r <= 100 and b <= 75 and g > r > b:
                tone = "Green-Brownish Tone"
            else:
                tone = "Other"            
            # Find matching color from CSV based on RGB values
            match_found = False
            color_name = ""
            for index, row in csv_data.iterrows():
                csv_color_name = row['Name']
                r_csv = row['R']
                g_csv = row['G']
                b_csv = row['B']
                
                # Define RGB range for color detection
                lower = np.array([r_csv - 20, g_csv - 20, b_csv - 20], dtype=np.uint8)
                upper = np.array([r_csv + 20, g_csv + 20, b_csv + 20], dtype=np.uint8)
                
                # Check if the clicked point matches this color
                if (lower[0] <= rgb_values[2] <= upper[0] and
                    lower[1] <= rgb_values[1] <= upper[1] and
                    lower[2] <= rgb_values[0] <= upper[2]):
                    color_name = csv_color_name
                    print(f"Color Name: {color_name}")
                    match_found = True
                    break
            
            if not match_found:
                print("No matching color found.")

            # Prepare text data for display
            text_data["rgb_text"] = f"RGB: {r}, {g}, {b}"
            text_data["color_name"] = color_name if match_found else ""
            text_data["tone"] = tone
            text_data["x"] = x
            text_data["y"] = y
            display_text_until = time.time() + 2  # Display text for 2 seconds

    cv2.namedWindow('Color Detection')
    cv2.setMouseCallback('Color Detection', mouse_callback)

    while True:
        ret, frame = cap.read()
        
        if not ret:
            print("Failed to capture image from webcam")
            break
        
        # Display text data if within the display duration
        if time.time() < display_text_until:
            cv2.putText(frame, text_data["rgb_text"], (text_data["x"] + 10, text_data["y"] + 10), 
                        cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 255, 255), 1, cv2.LINE_AA)
            if text_data["color_name"]:
                cv2.putText(frame, text_data["color_name"], (text_data["x"] + 10, text_data["y"] + 30), 
                            cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 255, 255), 1, cv2.LINE_AA)
            cv2.putText(frame, text_data["tone"], (text_data["x"] + 10, text_data["y"] + 50), 
                        cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 255, 255), 1, cv2.LINE_AA)

        # Display the resulting frame
        cv2.putText(frame, "Press q to quit", (10, frame.shape[0] - 10), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 255, 255), 1, cv2.LINE_AA)
        cv2.imshow('Color Detection', frame)
        
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break
    
    cap.release()
    cv2.destroyAllWindows()

if __name__ == "__main__":
    # Load CSV file
    csv_file = 'color_names.csv'
    df = pd.read_csv(csv_file)
    
    # Perform color detection
    detect_colors(df)
