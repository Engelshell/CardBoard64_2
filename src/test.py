from pathlib import Path
p = Path("/Volumes/Cardboard64/WCW-nWo Revenge (USA).z64")
p2 = Path("/Users/engelshell/Desktop/WCW-nWo Revenge (USA).z64")

'''
with open(p, "rb") as file1:
    # Reading from a file
    file1.seek(0x1FE00)
    bs = file1.read(0x200)
    for i in bs:
        print(hex(i), end=" ")
    print()
    print()
'''
with open(p, "rb") as file1:
    # Reading from a file
    file1.seek(0x20000)
    bs = file1.read(16)
    for i in bs:
        print(hex(i), end=" ")

    print()
    
'''
with open(p2, "rb") as file2:
    # Reading from a file
    file2.seek(0x20000)
    bs = file2.read(16)
    for i in bs:
        print(hex(i), end=" ")
    print()
'''