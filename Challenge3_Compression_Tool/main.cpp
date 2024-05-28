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
void encodeFile(std::string inputFileName, std::string outputFileName);
std::unordered_map<std::string, unsigned char>* createDecodings(std::unordered_map<unsigned char, std::string>* encodings);
void encode(std::unordered_map<unsigned char, std::string>* encodings, std::string filePath);
void decode(std::unordered_map<unsigned char, std::string>* decodings, std::string filePath);

int main(int argc, char* argv[])
{

    std::string filePath = ((argc > 1) ? argv[1] : "/home/vboxuser/CLionProjects/Compression_Tool/les_miserables.txt");
    std::string outputFile = ((argc > 2) ? argv[2] : "/home/vboxuser/CLionProjects/Compression_Tool/output.txt");

    // std::unordered_map<unsigned char, int>* counts = getCount(filePath);
    // hNode* huffmanTree = makeHuffmanTree(counts);
    // auto encodings = createEncodings(huffmanTree);

    encodeFile(filePath, outputFile);


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

void encodeFile(std::string inputFileName, std::string outputFileName) {
    // create a byte variable (char unsigned char, uint_8t)
    // that will hold our encodings. When it gets to 8 bits
    // we write the char (8 bits). This goes on until we have
    // written the whole file. If the last bits don't fit in
    // and you need to add padding, you can specify how much padding
    // is needed. We can also put this in the top within the freq
    // list. [ 'a' 1 'b' 2 'c' 3 6], there ar 3 key val pairs, then
    // there is a 6 there which means 6 padding bits at the end.
    // So we know the first byte is always the bracket, then we skip
    // spaces and read 2 words at a time, so we read key and val.
    // the last key val pair is incorrect and it is the padding bits
    // and closing bracket. Everything after that is data.
    std::unordered_map<unsigned char, int>* counts = getCount(inputFileName);
    hNode* huffmanTree = makeHuffmanTree(counts);
    auto encodings = createEncodings(huffmanTree);

    // use the encodings to map unsigned char to variable length encoding string.
    // turn that string into bits. Buffer (variable that is 8 bits) gets full, push
    // to file.

    FILE* inputFile = fopen(inputFileName.c_str(), "rb");
    FILE* outputFile = fopen(outputFileName.c_str(), "wb");


    // main loop, read bytes from input file -> use it to get corresponding
    // mapping from the map. -> convert that mapping to bits -> write bits
    // to file when they get to 8 -> continue till end. Might have to add the
    // padding byte in the end because its strange to go back to top and edit
    // (or can have random val there then edit it later)


    // write the amount of pairs incoming
    // reader now only needs to loop to
    // get all the key value pairs. Can
    // use some formatted string to get match.
    fprintf(outputFile, "%d", counts->size());
    for (auto  [ key, value ] : (*counts)) {
        // write counts to the output file
        fputc(key, outputFile);
        fputc(' ', outputFile);
        fprintf(outputFile, "%d", value);
        fputc(' ', outputFile);
    }


    unsigned char buffer[BUFFSIZE] = {0};
    int bytesRead = 0;
    uint8_t bitBuffer = 0;
    int increment = 0;

    while ((bytesRead = fread(buffer, sizeof(buffer[0]), BUFFSIZE, inputFile)) > 0) {

        for (int i = 0; i < bytesRead; i++) {
            // get mapping
            std::string mapping = (*encodings)[buffer[i]];
            for (int j = 0; j < mapping.size(); j++) {
                if (mapping[j] == '1') {
                    bitBuffer |= (1 << (7 - increment));
                }
                increment++;
                if (increment == 8) {
                    // write bitBuffer to file
                    // reset increment to 0
                    fputc(bitBuffer, outputFile);
                    increment = 0;
                    bitBuffer = 0;
                }
            }

        }
    }
    if (increment != 0) {
        // we have excess bits that do not fit into 8.
        // add them to the file, then add the next one
        // equal to the increment, which is the number of
        // valid bits in that last one.
        fputc(bitBuffer, outputFile);
        fprintf(outputFile, "%d", increment);
    }
    printf("Got this face\n");
    fclose(outputFile);
}
