# Broker that simulates a user-space virtual null modem.
# This script is necessary because COM handles are a headache to deal
# with on Windows, as it requires kernel-space authorization of drivers,
# and the use of COM handles is exclusive (if this script uses one COM
# device, no other application can connect/use it).

# Arguments to the script are:
# argv[1]: BMS handle name, including the \\.\pipe\ preemble
# argv[2]: APP handle name, including the \\.\pipe\ preemble

import win32pipe
import win32file
import threading
import time
import sys

VBMS = f"{sys.argv[1]}"
VAPP = f"{sys.argv[2]}"

def create_pipe_server(pipe_name):
	"""Creates a Named Pipe and waits for a client to connect."""
	print(f"Hosting {pipe_name}... waiting for client.")
	pipe = win32pipe.CreateNamedPipe(
		pipe_name,
		win32pipe.PIPE_ACCESS_DUPLEX,
		win32pipe.PIPE_TYPE_BYTE | win32pipe.PIPE_READMODE_BYTE | win32pipe.PIPE_WAIT,
		1, 65536, 65536, 0, None
	)
	win32pipe.ConnectNamedPipe(pipe, None)
	print(f"Client connected to {pipe_name}.")
	return pipe

def forward_data(pipe_in, pipe_out, name):
	"""Reads from pipe_in and write to pipe_out, hence it simulates a direct virtual connection in the vitual null modem."""
	try:
		while True:
			result, data = win32file.ReadFile(pipe_in, 4096)
			if result == 0 and data:
				win32file.WriteFile(pipe_out, data)
	except Exception as e:
		print(f"Connection lost on {name}: {e}")

def main():
	print("Starting Windows Virtual Null Modem (Broker)")

	h_bms = create_pipe_server(VBMS)
	h_app = create_pipe_server(VAPP)

	print("Both clients connected. Bridging data...")

	forward_thread = threading.Thread(target=forward_data, args=(h_bms, h_app, "BMS -> APP"))
	backward_thread = threading.Thread(target=forward_data, args=(h_app, h_bms, "APP -> BMS"))

	forward_thread.start()
	backward_thread.start()

	# keep the broker alive until interrupt
	try:
		while True:
			time.sleep(1)
	except KeyboardInterrupt:
		print("Shutting down broker...")

if __name__ == "__main__":
	main()