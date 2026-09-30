import struct, zlib
def write_png(path, w, h, rows, mode='L'):
    """rows: list of bytes objects of length w (mode L) or 3*w (mode RGB)."""
    ct = 0 if mode=='L' else 2
    raw = b''.join(b'\x00'+r for r in rows)
    def chunk(t, d):
        c = struct.pack('>I', len(d)) + t + d
        return c + struct.pack('>I', zlib.crc32(t+d) & 0xffffffff)
    png = b'\x89PNG\r\n\x1a\n' + chunk(b'IHDR', struct.pack('>IIBBBBB', w, h, 8, ct, 0, 0, 0)) \
        + chunk(b'IDAT', zlib.compress(raw, 9)) + chunk(b'IEND', b'')
    open(path,'wb').write(png)
