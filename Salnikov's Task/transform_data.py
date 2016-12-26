import numpy as np
import sys

def scan(file):
    data = []
    with open(file) as file_in:
        for line in file_in:
            data.append(line)
    return data

def erase_spaces_and_comments(data):
    data_fix = []
    for i in range(len(data)):
        if len(data[i]) == 0:
            continue
        last = len(data[i])
        for j in range(len(data[i])):
            if data[i][j][0] == '#':
                last = j
        if last == 0:
            continue
        data_fix.append(data[i][:last])
    return data_fix

def write_answer_to_file(answer,file):
    with open(file, 'w') as answer_file:
        for s in answer:
            answer_file.write(s + '\n')

state = 'state.txt'
state_fix = 'state_fix.txt'
creatures = 'creatures.conf'
creatures_fix = 'creatures_fix.conf'

data_state = erase_spaces_and_comments([s.split() for s in scan(state)])
date_creatures = erase_spaces_and_comments([s.split() for s in scan(creatures)])

data_str = []
data_str.append(
    str(data_state[0][1])
    + ' ' +  str(data_state[0][3])
    + ' ' + str(len(data_state) - 1))


for i in range(1,len(data_state)):
    for j in range(len(data_state[i])):
        data_str.append(data_state[i][j])

print(data_str)
write_answer_to_file(data_str, state_fix)

d = dict()
for s in date_creatures:
    d[s[0]] = s[2]

L = ['pollutionBound', 'divisionBound', 'vConsumeFeed',
'vConsumeStuff1', 'vConsumeStuff2', 'vProduceStuff1', 'vProduceStuff2']
data_str = [d[l] for l in L]
print(data_str)

write_answer_to_file(data_str, creatures_fix)
