#include <iostream>
bool inArray(char* array[], std::string elem, int len);

struct output {
    std::string type;
    int number;
};
int main(int argc, char* argv[]) {
    std::string filename = "";
    // how to determine if there is a file in the command line args?
    // So we can keep the statement below because if the length of argc
    // is greater than 1 there is a chance that there is a file in there.
    // the next thing we can do in the if statement is check if that last
    // input does not equal to -c -w -l -m, in that case wee keep it, otherwise
    // throw it away.
    if (argc > 1) {
        // file has to be last one
        std::string arg = std::string(argv[argc-1]);
        if (arg != "-c" && arg != "-w" && arg != "-m" && arg !=  "-l") {
            filename = arg;
        }
    }
    /*
     * get command line args (argc num args, argv the list of them)
     * opening files in c++ the standard way (fstream, getline,
     *
     *
     */
    // compares in c++ can be done with string class, or with strcmp
    // printf("%d is true\n", std::string(argv[1]) == "-c");


    // we want to check all of the folliwng:
    // if any on of these are in the args,
    // then we will count them, if none are
    // then we do all.
    // -c (COUNT BYTES)
    // -l (COUNT LINES)
    // -m (COUNT CHARS)
    // -w (COUNT WORDS)
    struct option {
        std::string symbol;
        bool enabled;
    };

    bool countChar = false, countWord = false, countByte = false, countLine = false;
    countChar = inArray(argv + 1, "-m", argc-1);
    countWord = inArray(argv + 1, "-w", argc-1);
    countLine = inArray(argv + 1, "-l", argc-1);
    countByte = inArray(argv + 1, "-c", argc-1);
    // if all are false, set to true
    if (!countWord && !countChar && !countLine && !countByte) {
        countWord = countByte = countChar = countLine = true;
    }
    FILE* fptr = (filename != "") ? fopen(filename.c_str(), "r") : stdin;

    if (fptr == NULL) {
        printf("The file did not open\n");
        return 0;
    }
    // we will eventually put conditions on these depending on the user's
    // input parameters, and if they are part of it, we will append to the
    // output string.

    // using fseek strategy only works for files, not pipes so  we cannot use fseek then
    // count bytes. Better to count it in the main loop with the other things.
    int totalBinarySize = 0;


    // to get the lines, its quite easy, any time there is a \n we got a word.
    // to get char we just count each char in there (include \n?)
    // To count words, any time there is a space and prev char isn't space

    char c = NULL;

    // setlocale(LC_ALL, "");

    int words = 0, lines = 0, chars = 0;
    bool inWord = false;
    while ((c = fgetc(fptr)) != EOF) {
        // for ascii characters we can increment chars
        // easily. But for other multibyte  sequences we
        // should only increment on the last of the bytes.
        // So depending on if it a 2 byte, 3 byte, or 4 byte
        // sequence I will call fgetc that many times. Only then
        // will I increment char count.

        // 4 byte sequence, skip next 3 (11110xxx -> compare with f8, make sure it equals 0xf)
        if ((c & 0xf8) == 0xf0) {
            fgetc(fptr);
            fgetc(fptr);
            fgetc(fptr);
            totalBinarySize += 3;
        }
        // 3 byte sequence, skip next 2 (1110xxxx -> compare with 0xf, make sure it equals 0xe)
        else if ((c & 0xf0) == 0xe0) {
            fgetc(fptr);
            fgetc(fptr);
            totalBinarySize += 2;
        }
        // 2 byte sequnce, skip next 1 (110xxxxx -> compare with 0xe, make sure it equals 0xc)
        else if ((c & 0xe0) == 0xc0) {
            fgetc(fptr);
            totalBinarySize += 1;
        }
        // finally increment
        chars++;
        totalBinarySize++;

        if (std::iswspace(c)) {
            inWord = false;
        }
        else if (!inWord) {
            words++;
            inWord = true;
        }
        if (c == '\n') {
            lines++;
        }


    }
    std::string outputString = "";
    output line = { "-l", lines };
    output word = { "-w", words };
    output character = { "-m", chars };
    output byte = { "-c", totalBinarySize };

    for (int i = 0; i < argc - 1; i++) {
        if (std::string(argv[1 + i]) == line.type && countLine) {
            outputString += (std::to_string(line.number) + "\t");
            continue;
        }
        if (std::string(argv[1 + i]) == word.type && countWord) {
            outputString += (std::to_string(word.number) + "\t");
            continue;
        }
        if (std::string(argv[1 + i]) == character.type && countChar) {
            outputString += (std::to_string(character.number) + "\t");
            continue;
        }
        if (std::string(argv[1 + i]) == byte.type && countByte) {
            outputString += (std::to_string(byte.number) + "\t");
        }

    }

    // it can be true for multiple reasons so if we have all 4 then
    // the above loop will run and so will this if, so we will get the
    // answer twice. (fix by only allowing this if statement to run if
    // there are no args except maybe the file. So if all count are true
    // weknow that the args do not include the -c -w -m -l or include all
    // of them. So all we have to do to make sure it doesn't include all of them
    // is to check if the arg size is less than that. (arg < 3) ensures we only
    // account for this file name, and the input filename if there is one.
    if (countByte && countWord && countChar && countLine && argc < 3) {
        outputString += (std::to_string(line.number) + "\t");
        outputString += (std::to_string(word.number) + "\t");
        outputString += (std::to_string(byte.number) + "\t");
        outputString += (std::to_string(character.number) + "\t");
    }

    std::cout  << outputString << std::endl;

    return 0;
}

bool inArray(char* arr[], std::string elem, int len) {
    for (int i = 0; i < len; i++) {
        if (std::string(arr[i]) == elem)
            return true;
    }
    return false;
}
