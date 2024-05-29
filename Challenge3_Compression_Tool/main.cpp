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
        if (a->count == b->count) {
            return (a->data < b->data);
        }
        return a->count > b->count;
    }
};
std::unordered_map<unsigned char, int>* getCount(std::string filePath);
hNode* makeHuffmanTree(std::unordered_map<unsigned char, int>* counts);

std::unordered_map<unsigned char, std::string>* createEncodings(hNode* huffmanTree);
void recursiveEncode(hNode* node, std::unordered_map<unsigned char, std::string>* encodings, std::string currEncoding);
void encodeFile(std::string inputFileName, std::string outputFileName);
std::unordered_map<std::string, unsigned char>* createDecodings(std::unordered_map<unsigned char, int>* counts);
void decodeFile(std::string outputFileName, std::string decodedFileName);

int main(int argc, char* argv[])
{
    if (argc == 1) {
        printf("No Input File!\n");
        return 1;
    }
    std::string filePath = argv[1];
    // std::string filePath = ((argc > 1) ? argv[1] : "/home/vboxuser/CLionProjects/Compression_Tool/test.txt");
    std::string outputFile = ((argc >2) ? argv[2] : "output.txt");
    std::string decodedFile = "decoded.txt";
    // std::unordered_map<unsigned char, int>* counts = getCount(filePath);
    // hNode* huffmanTree = makeHuffmanTree(counts);
    // auto encodings = createEncodings(huffmanTree);

    encodeFile(filePath, outputFile);
    decodeFile(outputFile, decodedFile);


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

    std::unordered_map<unsigned char, int>* counts = getCount(inputFileName);
    hNode* huffmanTree = makeHuffmanTree(counts);
    auto encodings = createEncodings(huffmanTree);

    // use the encodings to map unsigned char to variable length encoding string.
    // turn that string into bits. Buffer (variable that is 8 bits) gets full, push
    // to file.

    FILE* inputFile = fopen(inputFileName.c_str(), "rb");
    FILE* outputFile = fopen(outputFileName.c_str(), "wb");

    // fprintf(outputFile, "%d ", static_cast<int>(counts->size()));
    int numCount = counts->size();
    fwrite(&numCount, sizeof(int), 1, outputFile);
    for (const auto& [key, value] : (*counts)) {
        fputc(key, outputFile);
        fwrite(&value, sizeof(int), 1, outputFile);
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

std::unordered_map<std::string, unsigned char>* createDecodings(std::unordered_map<unsigned char, int>* counts) {
    auto tree = makeHuffmanTree(counts);
    auto encodings = createEncodings(tree);
    auto decodings = new std::unordered_map<std::string, unsigned char>;
    // insert into decodings : (flipped key val pairs)

    for (auto  [ key, value ] : (*encodings)) {
        decodings->insert({value, key});
    }

    return decodings;
}

void decodeFile(std::string encodedFileName, std::string outputFileName) {
    FILE* decodedFile = fopen(encodedFileName.c_str(), "rb");
    FILE* outputFile = fopen(outputFileName.c_str(), "wb");
    if (!decodedFile) {
        perror("Failed to open file for reading");
        return;
    }


    fseek(decodedFile,0, SEEK_END);
    long fileSize = ftell(decodedFile);

    // Move to the last byte
    fseek(decodedFile, -1, SEEK_END);

    // Read the last byte
    unsigned char lastByte;

    if (fread(&lastByte, sizeof(unsigned char), 1, decodedFile) != 1) {
        perror("Failed to read the last byte");
        fclose(decodedFile);
        return;
    }
    int nonPadded = lastByte - '0';

    fseek(decodedFile, 0, SEEK_SET);

    int numPairs;
    fread(&numPairs, sizeof(int), 1, decodedFile);

    unsigned char key;
    int value;
    std::unordered_map<unsigned char, int> counts;
    for (int i = 0; i < numPairs; ++i) {
        // Read the key and value
        unsigned char key = fgetc(decodedFile);
        int value;
        fread(&value, sizeof(int), 1, decodedFile);
        counts[key] = value;
    }

    // we technically want the total size minus the decodings. So we will subtract
    // the decodings we got from the total size. the file pointer should have the
    // size up until after the decodings. so subtract that.

    fileSize -= ftell(decodedFile);

    // get decodings
    auto decodings = createDecodings(&counts);

    uint8_t buffer[BUFFSIZE] = {0};
    long totalBytesRead = 0;
    int bytesRead = 0, bitPosition = 0;
    std::string bitString = "";
    while ((bytesRead = fread(buffer, sizeof(buffer[0]), BUFFSIZE, decodedFile)) > 0) {
       for (int i = 0; i < bytesRead; i++) {
            // if total bytes equals to the
           // second last byte, we can interpret that
            totalBytesRead++;
           if (totalBytesRead == fileSize) {
               // if we are on last byte, we can exit.
               break;
           }
           // if bit is found at that position add a 1
           while (true) {
               bitString += ((buffer[i] & (1 << (7 - bitPosition))) ? "1" : "0");
               if (decodings->find(bitString) != decodings->end()) {
                   // printf("THE FIRST ONE: %s decoded to %c\n", bitString.c_str(), (*decodings)[bitString]);
                   // exit(1);
                   fputc((*decodings)[bitString], outputFile);
                   bitString = "";
               }
               bitPosition++;
               // if we are on the second last bit
               // which could possibly be padded,
               // we cannot go past nonpadded bits.
               if (totalBytesRead == (fileSize - 1)) {
                   if (bitPosition >= nonPadded) {
                      break;
                   }
               }
               if (bitPosition == 8) {
                   bitPosition = 0;
                   break;
               }
           }


       }

    }

   fclose(outputFile);
}
