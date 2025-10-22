import os

def le16(n):
    return bytes((n & 0xFF, (n >> 8) & 0xFF))

def le32(n):
    return bytes((n & 0xFF, (n >> 8) & 0xFF, (n >> 16) & 0xFF, (n >> 24) & 0xFF))

def create_bootable_image():
    image_size = 1474560
    with open('axos.img', 'wb') as img:
        img.write(b'\x00' * image_size)
    if os.path.exists('boot.bin'):
        with open('boot.bin', 'rb') as boot:
            boot_data = boot.read()
        with open('axos.img', 'r+b') as img:
            img.seek(0)
            img.write(boot_data)
    else:
        print('Error: boot.bin not found!')
        return False
    if os.path.exists('kernel.bin'):
        with open('kernel.bin', 'rb') as kernel:
            kernel_data = kernel.read()
        with open('axos.img', 'r+b') as img:
            img.seek(512)
            img.write(kernel_data)
    else:
        print('Error: kernel.bin not found!')
        return False
    print('Bootable image created successfully!')
    return True

def create_fat12_image():
    sectors = 2880
    bps = 512
    spc = 1
    reserved = 1
    fats = 2
    root_entries = 16
    spf = 9
    total = sectors
    size = sectors * bps
    with open('fs.img', 'wb') as f:
        f.write(b'\x00' * size)
    bs = bytearray(bps)
    bs[0:3] = b'\xEB\x3C\x90'
    bs[3:11] = b'AXOS    '
    bs[11:13] = le16(bps)
    bs[13] = spc
    bs[14:16] = le16(reserved)
    bs[16] = fats
    bs[17:19] = le16(root_entries)
    bs[19:21] = le16(total)
    bs[21] = 0xF0
    bs[22:24] = le16(spf)
    bs[24:26] = le16(18)
    bs[26:28] = le16(2)
    bs[28:32] = le32(0)
    bs[32:36] = le32(0)
    bs[510] = 0x55
    bs[511] = 0xAA
    with open('fs.img', 'r+b') as f:
        f.seek(0)
        f.write(bs)
        fat = bytearray(spf * bps)
        fat[0] = 0xF0
        fat[1] = 0xFF
        fat[2] = 0xFF
        fat[3] = 0xFF
        fat[4] = 0x0F
        f.seek(bps * reserved)
        f.write(fat)
        f.seek(bps * (reserved + spf))
        f.write(fat)
        root_start = reserved + fats * spf
        root = bytearray(bps)
        name = b'HELLO   TXT'
        root[0:11] = name
        root[11] = 0x20
        root[26:28] = le16(2)
        data = b'Hello from FAT!\r\n'
        root[28:32] = le32(len(data))
        f.seek(bps * root_start)
        f.write(root)
        data_start = root_start + ((root_entries * 32 + (bps - 1)) // bps)
        cluster2_lba = data_start + (2 - 2) * spc
        f.seek(bps * cluster2_lba)
        f.write(data)
    print('FAT image created successfully!')
    return True

if __name__ == '__main__':
    ok = create_bootable_image()
    if ok:
        create_fat12_image()