# Neola : a software for soloist-orchestra synchronization

Neola is a software tool designed to help a soloist synchronize with a pre-recorded accompaniment. It allows the soloist to maintain full freedom in their interpretation while being "followed" by the orchestra.

## Overview

The main idea behind Neola is to define synchronization points where the orchestra should **wait** for the soloist and points where the orchestra should **resume playing**. For example, during a concerto, if the orchestra stops for a solo, you can set a "stop" sync point at that moment. Once the solo is finished, the soloist can press a USB pedal or a key on the computer to trigger a "start" sync point, and the orchestra will automatically resume playing at the right moment.  

This system ensures that the soloist can perform freely while keeping the accompaniment perfectly synchronized.

## Features

- Define **stop** and **start** synchronization points for the orchestra.  
- Control the orchestra with a computer keyboard or a USB pedal.  
- Perfect for concertos, solo performances, or any scenario where a live soloist is accompanied by a pre-recorded orchestra.  
- Flexible timing, allowing the soloist to take liberties with interpretation.  

## Usage

1. Load the pre-recorded accompaniment.  
2. Define synchronization points where the orchestra should stop and start.  
3. During the performance, press the pedal or a key to resume the orchestra at the appropriate moment.  


## Requirements

Programmed using Qt6.5.3 with MSVC2019 64bit
