# cs374_A5
One-Time Pad (OTP) Encryption System
Project Overview
This project implements a One-Time Pad encryption and decryption system. It operates using a client-server model where client programs send data and keys to background server programs via network sockets. The servers perform the cryptographic operations and return the results to the clients.

File Descriptions
1. Key Generation
keygen.c / keygen (executable):
This program creates a random sequence of characters (a key) of a specified length and outputs it. This key is required for the One-Time Pad algorithm to work.

2. The Encryption System
enc_server.c / enc_server (executable):
The encryption daemon. It runs in the background on a specified network port, listening for incoming connections from the encryption client. When it receives a plaintext message and a key, it performs the One-Time Pad encryption and sends the resulting ciphertext back to the client.

enc_client.c / enc_client (executable):
The client program that connects to enc_server. It reads a plaintext file and a key file, verifies that the key is at least as long as the plaintext, and sends both over a socket to the server. It then receives the encrypted ciphertext and outputs it.

3. The Decryption System
dec_server.c / dec_server (executable):
The decryption daemon. Similar to the encryption server, it runs in the background on a specified port. It receives ciphertext and a key from the decryption client, performs the OTP decryption, and returns the original plaintext.

dec_client.c / dec_client (executable):
The client program that connects to dec_server. It sends a ciphertext file and a key file to the server and outputs the decrypted plaintext it receives back.

4. Data and Test Files
plaintext1, plaintext1_a: Sample text files containing the original, readable messages to be encrypted.

mykey, myshortkey: Generated key files used for testing the encryption/decryption process. myshortkey is likely used to test error handling (e.g., when a key is too short for the provided plaintext).

ciphertext1: The encrypted output file generated after running the plaintext and key through the encryption system.
