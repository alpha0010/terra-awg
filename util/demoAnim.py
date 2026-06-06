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

frames = [{} for _ in range(120)]

def interpVar(varName, varStart, varStop, frameStart, frameStop):
    varDist = varStop - varStart
    frameDist = frameStop - frameStart - 1
    for frame in range(frameStart, frameStop):
        frames[frame][varName] = varStart + ((frame - frameStart) / frameDist) * varDist

def fillVar(varName, value, frameStart, frameStop):
    for frame in range(frameStart, frameStop):
        frames[frame][varName] = value

interpVar('snowSize', 1.0, 3.0, 0, 30)
fillVar('shattered', True, 25, 35)

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
    if idx == 30:
        break
