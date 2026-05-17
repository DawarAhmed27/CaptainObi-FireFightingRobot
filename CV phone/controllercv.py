import socket
import cv2
import pygame
import time
from ultralytics import YOLO
from pathlib import Path
import sys

ip = "172.20.10.4"
p = 8080
sk = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

m_p = Path(__file__).with_name('best.pt')
if not m_p.exists():
    sys.exit(1)

try:
    mdl = YOLO(str(m_p))
except Exception:
    sys.exit(1)

pygame.init()
pygame.joystick.init()
js = None

if pygame.joystick.get_count() > 0:
    js = pygame.joystick.Joystick(0)
    js.init()

c = cv2.VideoCapture(0)
if not c.isOpened():
    c = cv2.VideoCapture(0)
if not c.isOpened():
    sys.exit(1)

st_th = 0.03
md = 'auto'
l_cmd = ''
l_btn = False

# Turret Sweep Variables 
swp_dir = 'r'
swp_tm = time.time()

while True:
    r, f = c.read()
    if not r: break

    cmd = 's'
    w = f.shape[1] 
    
    if js:
        pygame.event.pump()
        c_btn = js.get_button(3)
        
        if c_btn and not l_btn: 
            md = 'man' if md == 'auto' else 'auto'
            try:
                sk.sendto('s'.encode(), (ip, p))
                l_cmd = 's'
            except: pass
        
        l_btn = c_btn

    if md == 'man' and js:
        cv2.putText(f, "MAN", (30, 50), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 255), 3)
        ax_x = js.get_axis(0)
        ax_y = js.get_axis(1)
        
        if ax_y < -0.5: cmd = 'f'
        elif ax_y > 0.5: cmd = 'b'
        elif ax_x < -0.5: cmd = 'l'
        elif ax_x > 0.5: cmd = 'r'
        
        if js.get_button(0): cmd = 'x'
        
    else:
        cv2.putText(f, "AUTO", (30, 50), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 3)
        res = mdl(f, verbose=False)
        
        f_f = False 

        for x in res:
            bx = x.boxes
            for y in bx:
                try: cls = int(y.cls[0])
                except: cls = int(y.cls)
                
                try: nm = mdl.names[int(cls)]
                except: nm = getattr(getattr(mdl, 'model', None), 'names', {})[int(cls)]
                
                if nm == 'fire':
                    f_f = True 
                    xy = y.xyxy[0]
                    try: x1, y1, x2, y2 = float(xy[0]), float(xy[1]), float(xy[2]), float(xy[3])
                    except: x1, y1, x2, y2 = map(float, xy)
                    
                    s_a = f.shape[1] * f.shape[0]  
                    f_a = (x2 - x1) * (y2 - y1)      
                    f_r = f_a / s_a
                    
                    cx = (x1 + x2) / 2
                    
                    #  Remember where the fire was last seen 
                    swp_dir = 'l' if cx < w / 2 else 'r'
                    swp_tm = time.time() 

                    if f_r >= st_th:
                        cmd = 'x'  
                        cv2.rectangle(f, (int(x1), int(y1)), (int(x2), int(y2)), (255, 0, 0), 4) 
                    else:
                        # 50% WIDE CENTER ZONE
                        if cx < w / 4: cmd = 'l'
                        elif cx > 3 * (w / 4): cmd = 'r'
                        else: cmd = 'f'
                        
                        cv2.rectangle(f, (int(x1), int(y1)), (int(x2), int(y2)), (0, 0, 255), 2)
                    break 
            if f_f: break 

        if not f_f:
            # TURRET SWEEP LOGIC 
            # If 3 seconds pass without finding fire, reverse sweep direction
            if time.time() - swp_tm > 3.0: 
                swp_dir = 'l' if swp_dir == 'r' else 'r'
                swp_tm = time.time()
            
            cmd = swp_dir 
            cv2.putText(f, f"SWEEPING {swp_dir.upper()}", (30, 90), cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 165, 255), 3)

    # CONTINUOUS UDP SPAM 
    try:
        sk.sendto(cmd.encode(), (ip, p))
        if cmd != l_cmd:
            print(f"Sent: {cmd}") 
            l_cmd = cmd
    except Exception:
        pass

    #  LIVE DEBUG TEXT 
    cv2.putText(f, f"CMD: {cmd}", (30, 130), cv2.FONT_HERSHEY_SIMPLEX, 1, (255, 0, 255), 3)

    cv2.imshow('v', f)
    if cv2.waitKey(1) == ord('q'): break

c.release()
pygame.quit()
cv2.destroyAllWindows()