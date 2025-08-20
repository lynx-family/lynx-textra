# Copyright 2021 The Lynx Authors. All rights reserved.
# Licensed under the Apache License Version 2.0 that can be found in the
# LICENSE file in the root directory of this source tree.

test_1 = []
test_2 = []
test_4 = []

level_1 = []
level_2 = []
level_4 = []

reorder_1 = []
reorder_2 = []
reorder_4 = []

Dict = {
    "L":'A',
    "R":'B',
    "AL":'C',
    "EN":'D',
    "AN":'E',
    "ES":'F',
    "ET":'G',
    "CS":'H',
    "NSM":'I',
    "BN":'J',
    "B":'K',
    "S":'L',
    "WS":'M',
    "ON":'N',
    "LRE":'O',
    "RLE":'P',
    "LRO":'Q',
    "RLO":'R',
    "PDF":'S',
    "LRI":'T',
    "RLI":'U',
    "FSI":'V',
    "PDI":'W'
}

def parseLevels(line):
    lst = line.split(':	')[1]
    return lst

def parseReorder(line):
    lst = line.split(':	')[1]
    return lst

def parseTestLine(line):
    lst = line.split(';')
    contents = lst[0].split(' ')
    content = []
    for con in contents:
        content.append(Dict[con])
    bitsets = lst[1]
    return content, int(bitsets)

def detectLineType(line):
    if(len(line) == 0 or line[0] == '#' or line[0] == '\n'):
        return -1
    if(line[0] == '@' and line[1] == 'L'):
        return 1
    if(line[0] == '@' and line[1] == 'R'):
        return 2
    return 3

levels = []
reorder = []

with open("BidiTest.txt", "r") as file:
    ind = 0
    line = file.readline()
    while line:
        print(ind)
        ind += 1
        line_type = detectLineType(line)
        if(line_type == 1):
            levels = parseLevels(line)
        if(line_type == 2):
            reorder = parseReorder(line)
        if(line_type == 3):
            cont, bits = parseTestLine(line)
            if(bits == 1):
                test_1.append(cont)
                level_1.append(levels)
                reorder_1.append(reorder)
            if(bits == 2):
                test_2.append(cont)
                level_2.append(levels)
                reorder_2.append(reorder)
            if(bits == 3):
                test_1.append(cont)
                level_1.append(levels)
                reorder_1.append(reorder)
                test_2.append(cont)
                level_2.append(levels)
                reorder_2.append(reorder)
            if(bits == 4):
                test_4.append(cont)
                level_4.append(levels)
                reorder_4.append(reorder)
            if(bits == 5):
                test_1.append(cont)
                level_1.append(levels)
                reorder_1.append(reorder)
                test_4.append(cont)
                level_4.append(levels)
                reorder_4.append(reorder)
            if(bits == 6):
                test_2.append(cont)
                level_2.append(levels)
                reorder_2.append(reorder)
                test_4.append(cont)
                level_4.append(levels)
                reorder_4.append(reorder)
            if(bits == 7):
                test_1.append(cont)
                level_1.append(levels)
                reorder_1.append(reorder)
                test_2.append(cont)
                level_2.append(levels)
                reorder_2.append(reorder)
                test_4.append(cont)
                level_4.append(levels)
                reorder_4.append(reorder)
        line = file.readline()

with open("BidiTest.1.test.txt", "w") as file:
    for line in test_1:
        l = ""
        for num in line:
            l+=str(num)
            l+="/"
        file.write(l + '\n')

with open("BidiTest.1.levels.txt", "w") as file:
    for line in level_1:
        file.write(line)

with open("BidiTest.1.reorder.txt", "w") as file:
    for line in reorder_1:
        file.write(line)

with open("BidiTest.2.test.txt", "w") as file:
    for line in test_2:
        l = ""
        for num in line:
            l+=str(num)
            l+="/"
        file.write(l + '\n')

with open("BidiTest.2.levels.txt", "w") as file:
    for line in level_2:
        file.write(line)

with open("BidiTest.2.reorder.txt", "w") as file:
    for line in reorder_2:
        file.write(line)

with open("BidiTest.4.test.txt", "w") as file:
    for line in test_4:
        l = ""
        for num in line:
            l+=str(num)
            l+="/"
        file.write(l + '\n')

with open("BidiTest.4.levels.txt", "w") as file:
    for line in level_4:
        file.write(line)

with open("BidiTest.4.reorder.txt", "w") as file:
    for line in reorder_4:
        file.write(line)


        
