#!/usr/bin/env python3

import sys
import textwrap

try:
    from natsort import natsorted
except ModuleNotFoundError:
    natsorted = sorted

def readUint8(f):
    return int.from_bytes(f.read(1))

def readUint16(f):
    return int.from_bytes(f.read(2), byteorder='little')

def readUint32(f):
    return int.from_bytes(f.read(4), byteorder='little')

def readString(f):
    # LEB128 encoded length prefix.
    strLen = 0
    shift = 0
    while True:
        b = readUint8(f)
        strLen |= (b & 0x7f) << shift
        if (b & 0x80) == 0:
            break
        shift += 7
    return f.read(strLen).decode('utf-8')

def readBitVec(f):
    vecLen = readUint16(f)
    b = 0
    mask = 0x80
    bitVec = []
    for i in range(vecLen):
        if mask == 0x80:
            b = readUint8(f)
            mask = 0x01
        else:
            mask <<= 1
        bitVec.append((b & mask) == mask)
    return bitVec

def loadTiles(filename):
    with open(filename, mode='rb') as f:
        print('//', readString(f))
        version = readUint32(f)
        assert version == 10279, f'Unknown file version {version}'
        framedTiles = readBitVec(f)
        width = readUint32(f)
        height = readUint32(f)
        assert width < 0xff and height < 0xff, f'Structure too large {width}x{height}'
        tiles = [[{} for y in range(height)] for x in range(width)]
        for x in range(width):
            rle = 0
            for y in range(height):
                if rle > 0:
                    tiles[x][y] = tiles[x][y - 1]
                    rle -= 1
                else:
                    flags = [0, 0, 0, 0]
                    for i in range(len(flags)):
                        flags[i] = readUint8(f)
                        if flags[i] & 0x01 == 0:
                            break
                    if flags[0] & 0x02 == 0:
                        tiles[x][y]['blockID'] = -1
                    else:
                        if flags[0] & 0x20 == 0:
                            tiles[x][y]['blockID'] = readUint8(f)
                        else:
                            tiles[x][y]['blockID'] = readUint16(f)
                        if framedTiles[tiles[x][y]['blockID']]:
                            tiles[x][y]['frameX'] = readUint16(f)
                            tiles[x][y]['frameY'] = readUint16(f)
                        if flags[2] & 0x08 == 0x08:
                            tiles[x][y]['blockPaint'] = readUint8(f)
                        tiles[x][y]['slope'] = (flags[1] >> 4) & 0x07
                    if flags[0] & 0x04 == 0:
                        tiles[x][y]['wallID'] = 0
                    else:
                        tiles[x][y]['wallID'] = readUint8(f)
                        if flags[2] & 0x10 == 0x10:
                            tiles[x][y]['wallPaint'] = readUint8(f)
                    if flags[0] & 0x18 == 0x08:
                        if flags[2] & 0x80 == 0x80:
                            tiles[x][y]['liquid'] = 'shimmer'
                        else:
                            tiles[x][y]['liquid'] = 'water'
                    elif flags[0] & 0x18 == 0x10:
                        tiles[x][y]['liquid'] = 'lava'
                    elif flags[0] & 0x18 == 0x18:
                        tiles[x][y]['liquid'] = 'honey'
                    if 'liquid' in tiles[x][y]:
                        readUint8(f) # Liquid amount.
                    else:
                        tiles[x][y]['liquid'] = 'none'
                    if flags[2] & 0x40 == 0x40:
                        tiles[x][y]['wallID'] |= readUint8(f) << 8
                    tiles[x][y]['wireRed'] = flags[1] & 0x02 == 0x02
                    tiles[x][y]['wireBlue'] = flags[1] & 0x04 == 0x04
                    tiles[x][y]['wireGreen'] = flags[1] & 0x08 == 0x08
                    tiles[x][y]['wireYellow'] = flags[2] & 0x20 == 0x20
                    tiles[x][y]['actuator'] = flags[2] & 0x02 == 0x02
                    tiles[x][y]['actuated'] = flags[2] & 0x04 == 0x04
                    tiles[x][y]['echoCoatBlock'] = flags[3] & 0x02 == 0x02
                    tiles[x][y]['echoCoatWall'] = flags[3] & 0x04 == 0x04
                    tiles[x][y]['illuminantBlock'] = flags[3] & 0x08 == 0x08
                    tiles[x][y]['illuminantWall'] = flags[3] & 0x10 == 0x10
                    if flags[0] & 0x40 == 0x40:
                        rle = readUint8(f)
                    elif flags[0] & 0x80 == 0x80:
                        rle = readUint16(f)
        return (tiles, framedTiles)

# header(16)
# blockID(16) frameX(16) frameY(16) wallID(16)
# blockPaint(8) wallPaint(8) liquid(3) slope(3)
# wires,actuators,coating(10)
def serializeTile(tile, framedTiles):
    for key in ['slope', 'blockPaint', 'wallPaint']:
        if not key in tile:
            tile[key] = 0
    data = [0]
    if tile['blockID'] != -1:
        data[0] |= 0x1000
        data.append(tile['blockID'])
        if framedTiles[tile['blockID']]:
            data.append(tile['frameX'])
            data.append(tile['frameY'])
    if tile['wallID'] != 0:
        data[0] |= 0x2000
        data.append(tile['wallID'])
    if tile['blockPaint']  != 0 or tile['wallPaint'] != 0:
        data[0] |= 0x4000
        data.append(tile['blockPaint'] | (tile['wallPaint'] << 8))
    flags = {
        'none': 0,
        'water': 1,
        'lava': 2,
        'honey': 3,
        'shimmer': 4,
    }[tile['liquid']]
    flags |= tile['slope'] << 3
    flagDef = {
        'wireRed': 6,
        'wireBlue': 7,
        'wireGreen': 8,
        'wireYellow': 9,
        'actuator': 10,
        'actuated': 11,
        'echoCoatBlock': 12,
        'echoCoatWall': 13,
        'illuminantBlock': 14,
        'illuminantWall': 15,
    }
    for key, shift in flagDef.items():
        if tile[key]:
            flags |= 1 << shift
    if flags != 0:
        data[0] |= 0x8000
        data.append(flags)
    return data

for filename in natsorted(sys.argv[1:]):
    (tiles, framedTiles) = loadTiles(filename)
    data = [len(tiles) << 8 | len(tiles[0])]
    prevTile = []
    rleIndex = len(data)
    for column in tiles:
        for tile in column:
            curTile = serializeTile(tile, framedTiles)
            if prevTile == curTile:
                data[rleIndex] += 1
            else:
                rleIndex = len(data)
                data += curTile
                prevTile = curTile
    print('{')
    print('\n'.join(textwrap.wrap(
        ', '.join([str(val) for val in data]),
        width=79,
        initial_indent='    ',
        subsequent_indent='    '
    )))
    print('},')
