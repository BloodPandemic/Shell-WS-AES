# Shell-WS-AES
Rev Shell on cpp , added some av_detection bypass methods from cocomelonc. just a PoC

so as u can see pretty much, The same from cocomelonc however he adds at the end of the article that we can encrypt(XOR) functions like "CreateProcess".
i Didn't just do it for "CreateProcess" "WSAConnect" and "WSAStartup". 
the aesEncrypt.py script , encrypts the "cmd.exe" command and the key , and then replaces them and creates a new file with the result.


