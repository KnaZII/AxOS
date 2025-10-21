"""
Скрипт для создания правильного загрузочного образа
"""

import os
import sys

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
        print("Error: boot.bin not found!")
        return False
    
    if os.path.exists('kernel.bin'):
        with open('kernel.bin', 'rb') as kernel:
            kernel_data = kernel.read()
            
        with open('axos.img', 'r+b') as img:
            img.seek(512)
            img.write(kernel_data)
    else:
        print("Error: kernel.bin not found!")
        return False
    
    print("Bootable image created successfully!")
    return True

if __name__ == "__main__":
    create_bootable_image()