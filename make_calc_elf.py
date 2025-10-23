import struct
with open('calc.bin','rb') as f:
    bin_data=f.read()
EHDR_SIZE=52
PHDR_SIZE=32
p_offset=0x1000
p_vaddr=0x00100000
entry=p_vaddr
hdr=bytearray(EHDR_SIZE)
hdr[0:4]=b'\x7FELF'
hdr[4]=1
hdr[5]=1
hdr[6]=1
hdr[7]=0
hdr[8:16]=bytes(8)
struct.pack_into('<HHIIIIIHHHHHH',hdr,16,2,3,1,entry,EHDR_SIZE,0,0,EHDR_SIZE,PHDR_SIZE,1,0,0,0)
phdr=bytearray(PHDR_SIZE)
struct.pack_into('<IIIIIIII',phdr,0,1,p_offset,p_vaddr,0,len(bin_data),len(bin_data),0x7,0x1000)
pad=b'\x00'*(p_offset-(EHDR_SIZE+PHDR_SIZE))
with open('calc.elf','wb') as f:
    f.write(hdr)
    f.write(phdr)
    f.write(pad)
    f.write(bin_data)