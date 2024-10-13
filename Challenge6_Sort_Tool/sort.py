import sys
from heapq import heapify, heappush, heappop

# take in an input and then sort it in terms of all words in there

# read each word from the file into the array


n = len(sys.argv)
filename = ""
options = {
    '-u': False
}
for i in range(1, n):
    if sys.argv[i][len(sys.argv[i])-4:] == '.txt':
        filename = sys.argv[i] 
    if sys.argv[i] in options:
        options[sys.argv[i]] = True

if filename == "":
    print("DID NOT ENTER FILE!")
    sys.exit()

words_array = []
# is there a way to sort the stream as it comes in instead of putting it into
# an array first? Lets think of that later. For now everything goes into the array.

with open(filename, 'r') as input_file:
    for line in input_file:
        words = line.split()
        words_array += words

if options['-u']:
    words_array = list(set(words_array))

# print(words_array)
def merge_sort(array, l, r):
    if l == r:
        return [array[l]]
    m = (l + r) // 2
    left = merge_sort(array, l, m)
    right = merge_sort(array, m+1, r)

    # sort the merged halves
    output = []
    p1, p2 = 0, 0
    while p1 < len(left) and p2 < len(right):
        if left[p1] <= right[p2]:
            output.append(left[p1])
            p1 += 1
        else:
            output.append(right[p2])
            p2 += 1

    while p1 < len(left):
        output.append(left[p1])
        p1 += 1
        
    while p2 < len(right):
        output.append(right[p2])
        p2 += 1
    return output


def heapsort(array):
    output = []
    heapify(array)
    while len(array):
        output.append(heappop(array))

    return output

# output = (merge_sort(words_array, 0, len(words_array)-1))
output = heapsort(words_array)
output_str = "\n".join(output)

print(output_str)

# arr = [9,7,3,1,4,6,2, 5]

# print(merge_sort(arr, 0, len(arr)-1))

