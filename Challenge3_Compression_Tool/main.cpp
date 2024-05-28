#include <cstring>
#include <iostream>
#include <queue>
#include <unordered_map>
#include <vector>

#define BUFFSIZE 512

/*
 * Plan for the remaining portion:
 * Allow the program to take in input and output file names
 * Include char frequencies in the output file for decoding
 * array brackets around the char key and frequencies with
 * space separating the char and frequency (key  val pair)
 * Use that to decode.
 *
 * The encoding should be simple. For each char, use the encoding
 * map to determine the output bit sequence. That will go into the array.
 * First you can write to a buffer, and then push that to file because
 * we need to turn the string sequence into bit sequence and the issue is
 * that it doesn't fit into exact byte sequences (since we have varying length)
 * Also we can see if there is a write bit? i doubt it. But still writing bits
 * to buffer shouldn't be too bad. But what do we do about the end of file not
 * being an exact byte. If we want to write 3 bits but we need to write at minimum
 * a byte at a time.
 *
 *
 *
 */

struct hNode {
    unsigned char data;
    int count;
    hNode* left;
    hNode* right;
};

class Compare
{
public:
    bool operator() (hNode* a, hNode* b)
    {
        return a->count > b->count;
    }
};
std::unordered_map<unsigned char, int>* getCount(std::string filePath);
hNode* makeHuffmanTree(std::unordered_map<unsigned char, int>* counts);

std::unordered_map<unsigned char, std::string>* createEncodings(hNode* huffmanTree);
void recursiveEncode(hNode* node, std::unordered_map<unsigned char, std::string>* encodings, std::string currEncoding);

std::unordered_map<std::string, unsigned char>* createDecodings(std::unordered_map<unsigned char, std::string>* encodings);
void encode(std::unordered_map<unsigned char, std::string>* encodings, std::string filePath);
void decode(std::unordered_map<unsigned char, std::string>* decodings, std::string filePath);

int main()
{
    std::string filePath = "/home/vboxuser/CLionProjects/Compression_Tool/test.txt";
    std::unordered_map<unsigned char, int>* counts = getCount(filePath);
    hNode* huffmanTree = makeHuffmanTree(counts);
    auto encodings = createEncodings(huffmanTree);

    return 0;
}

std::unordered_map<unsigned char, int>* getCount(std::string filePath) {
    // read in chunks, so use buffer.
    unsigned char buff[BUFFSIZE] = {0};

    // allocate new map to return with all of the counts
    auto count = new std::unordered_map<unsigned char, int>;
    FILE* fileptr = fopen(filePath.c_str(), "rb");
    if (!fileptr) {
        // Print detailed error message
        std::cerr << "Error opening file: " << filePath << std::endl;
        std::cerr << "Error: " << strerror(errno) << std::endl;
        return 0;
    }
    size_t bytesRead = 0;
    while ((bytesRead = fread(buff, sizeof(buff[0]), BUFFSIZE, fileptr)) > 0) {
        // if the byte value doesn't exist in the unordered map,
        // add it, if it does exist, increment it by 1.
        for (int i = 0; i < bytesRead; i++) {
            if (count->find(buff[i]) != count->end()) {
                // dereference, access the key, and icrement value
                (*count)[buff[i]]++;
            }
            else {
                count->insert({buff[i], 1});
            }
        }
    }

    return count;
}

hNode* makeHuffmanTree(std::unordered_map<unsigned char, int>* counts) {
    // use the count map to create hNodes with key as the data, and
    // count as the sum.
    std::vector<hNode*> nodes;
    for (auto  [ key, value ] : (*counts)) {
        nodes.push_back(new hNode{key, value, nullptr, nullptr});
    }

    // after getting all of these, feed it to the priority queue, but have your
    // custom compare function, and keep combining until you have one node left.
    std::priority_queue<hNode*, std::vector<hNode*>, Compare> pQueue;

    for (int i = 0; i < nodes.size(); i++) {
        pQueue.push(nodes[i]);
    }

    while (pQueue.size() > 1) {
        // get top two elements, combine
        hNode* a = pQueue.top();
        pQueue.pop();
        hNode* b = pQueue.top();
        pQueue.pop();

        pQueue.push(new hNode{NULL, a->count + b->count, a, b});
    }

    // one elemnent left which is the tree.
    return pQueue.top();
}

void recursiveEncode(hNode* node, std::unordered_map<unsigned char, std::string>* encodings, std::string currEncoding) {
        // if we are not at leaf node, increment the string
        // and recursively go to left and right, else add to map.
    if (node) {
        if (!node->left && !node->right) {
            // leaf node
            encodings->insert({node->data, currEncoding});
        }
        else {
            if (node->left)
                recursiveEncode(node->left, encodings, currEncoding + "0");
            if (node->right)
                recursiveEncode(node->right, encodings, currEncoding + "1");
        }
    }

}

std::unordered_map<unsigned char, std::string>* createEncodings(hNode* huffmanTree) {
    // take the huffman tree and then make encodings for each char.

    auto encodings = new std::unordered_map<unsigned char, std::string>;
    recursiveEncode(huffmanTree, encodings, "");

    // print encodings just to make sure
    for (const auto & [ key, value ] : (*encodings)) {
        printf("entry: %c : %s\n", (char)key, value.c_str());
    }
    return encodings;
}

