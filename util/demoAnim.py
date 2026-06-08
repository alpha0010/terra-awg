#!/usr/bin/env python3

from configparser import ConfigParser
import subprocess
from textwrap import wrap
import time

pristine = ConfigParser()
pristine.read('../terra-awg.ini')
pristine['world']['seed'] = 'DemoAnim'

def prepareConf(frame):
    config = ConfigParser()
    config.read_dict(pristine)
    for key, value in frame.items():
        config['variation'][key] = str(value)
    with open('scratch/terra-awg.ini', 'w') as f:
        config.write(f)

frames = [{} for _ in range(460)]

def interpVar(varName, varStart, varStop, frameStart, frameStop, asInt=False):
    varDist = varStop - varStart
    frameDist = frameStop - frameStart - 1
    for frame in range(frameStart, frameStop):
        interp = varStart + ((frame - frameStart) / frameDist) * varDist
        if asInt:
            frames[frame][varName] = int(interp)
        else:
            frames[frame][varName] = interp

def fillVar(varName, value, frameStart, frameStop):
    for frame in range(frameStart, frameStop):
        frames[frame][varName] = value

interpVar('snowSize', 1.0, 3.0, 0, 60)
interpVar('snowSize', 3.0, 1.0, 60, 90)
interpVar('desertSize', 1.0, 0.5, 22, 37)
interpVar('desertSize', 0.5, 1.0, 37, 67)
interpVar('livingTrees', 1.0, 3.5, 45, 90)
interpVar('livingTrees', 3.5, 1.0, 90, 135)
interpVar('evilSize', 1.0, 8.0, 75, 180)
fillVar('bothEvils', True, 105, 127)
interpVar('dungeonSize', 1.0, 2.5, 120, 165)
interpVar('dungeonSize', 2.5, 0.5, 165, 210)
interpVar('dungeonSize', 0.5, 1.0, 210, 232)
fillVar('dontDigUp', True, 180, 225)
fillVar('bothEvils', True, 195, 210)
fillVar('biomes', 'layers', 225, 235)
fillVar('biomes', 'patches', 235, 335)
interpVar('patchesTemperature', 0.0, -0.5, 245, 275)
interpVar('patchesTemperature', -0.5, 0.75, 275, 335)
interpVar('patchesHumidity', 0.0, 0.5, 260, 280)
interpVar('patchesHumidity', 0.5, 0.0, 280, 310)
interpVar('meteorites', 1, 45, 310, 335, asInt=True)
fillVar('hardmode', True, 325, 355)
interpVar('lakeSize', 1.0, 3.0, 335, 365)
interpVar('oceanSize', 1.0, 1.8, 350, 365)
fillVar('sunken', True, 365, 395)
interpVar('clouds', 1.0, 4.0, 370, 400)
interpVar('clouds', 4.0, 1.0, 400, 430)
fillVar('shattered', True, 380, 410)
fillVar('celebration', True, 405, 435)
interpVar('glowingMushroomSize', 1.0, 2.0, 415, 435)
fillVar('hiveQueen', True, 430, 460)
fillVar('dontDigUp', True, 450, 460)

def formatLabel(frame):
    label = []
    for key, value in sorted(frame.items()):
        if isinstance(value, float):
            label.append(f'{key}={value:.3f}')
        else:
            label.append(f'{key}={value}')
    return "\\n".join(wrap(' '.join(label), width=46))

startTime = time.perf_counter()
for idx, frame in enumerate(frames):
    runTime = (time.perf_counter() - startTime) / 60
    estTime = runTime * len(frames) / (idx + 1)
    print(f'{idx + 1}/{len(frames)}  {runTime:.2f}m/{estTime:.2f}m')
    label = formatLabel(frame)
    print(label)
    prepareConf(frame)
    subprocess.run('../../build/terra-awg', cwd='scratch', stdout=subprocess.DEVNULL)
    subprocess.run([
        'convert',
        'scratch/Terra_AWG_World-map.png',
        '-filter', 'box',
        '-resize', '25%',
        '-background', 'white',
        '-splice', '0x250',
        '-font', 'Ubuntu-Mono-Regular',
        '-pointsize', '60',
        '-annotate', '+20+80', label,
        f'out/frame-{idx:04d}.png',
    ])
