import websockets
import asyncio
import cv2
import numpy as np

async def connect_websocket():
    async with websockets.connect("ws://cobot.center:8286/pang/ws/sub?channel=instant&name=dGVzdA==&track=<label>", ping_interval=None) as ws:
        while True:
            img_binary_data = await ws.recv()
            encoded_img = np.frombuffer(img_binary_data, dtype = np.uint8)
            img = cv2.imdecode(encoded_img, cv2.IMREAD_COLOR)
            cv2.imshow("esp32cam", img)
            if cv2.waitKey(1) & 0xFF == ord('q'):
                cv2.destroyAllWindows()
                break  
def main():
    asyncio.run(connect_websocket())

if __name__ == '__main__':
    main()
    

