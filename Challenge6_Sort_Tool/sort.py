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

# this sort involves counting repeatedly for as many digits as the longest item
# it does a repetetive bucket sort, starting from the smallest digit to the largest
# digit. On subsequent iterations, the sort on the current digit will still keep any 
# items that were in the same group for the previous and current iteration in the same
# order. So an item in the nth group and the other in the n + 1th group in the previous
# iteration but in the current one they have the same digit, then we will still order them
# by the one that was in the nth group in the last iteration then the n + 1th item.

# 159, and 139 for example. In the first sort they both belong to the 9 group [159, 139]
# in the second iteration we see that one is in the 3 group [139], the other is in the 5 [159]
# next they will both go into the 1 group, but since we use the previous groupings and we start
# from index 0 to 9, we will come across 139 first, so it is placed in the 1 index [139], then we
# will find nothing in the 4 index, then the 5 index has 159 so we take it next. But see that the 
# previous groupings influence our current sorting. So then 159 is added [139, 159], so we have a fully
# sorted item.

# what happens when we have only integers, we can do 0-9
# if we have only lower case letters or only upper case 0-25
# if we have both then 0 - 51
# if ascii, then 0 - 127 i think. if its a file (best to play it safe and do ascii)
def radixsort(array):

    return 0   
# output = (merge_sort(words_array, 0, len(words_array)-1))
output = heapsort(words_array)
output_str = "\n".join(output)

print(output_str)

# arr = [9,7,3,1,4,6,2, 5]

# print(merge_sort(arr, 0, len(arr)-1))

