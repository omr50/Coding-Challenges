#include <iostream>
#include <vector>

#define BUFFERSIZE 512
// naive function that looks for dot
// if dot in arg, then it is a file name
bool containsFileExtensions(std::string filename) {
    for (int i = 0; i < filename.size(); i++) {
        if (filename[i] == '.')
            return true;
    }
    return false;
}

// field list
std::vector<int> getFieldList(std::string fieldParam) {

    std::vector<int> fieldList;
    // -f"1 2" would be interpretted as -f1 2 as a string

    // three cases: -f1 -f"1 2" or -f1,2

    // we can check if there are commas or spaces.
    // replace all commas with spaces to make it
    // uniform format. If no comma or space just return
    // the end of it converted to int.
    bool noSpaceComma = true;
    for (int i = 0; i < fieldParam.size(); i++) {
        if (fieldParam[i] == ' ' || fieldParam[i] == ',') {
            noSpaceComma = false;
            if (fieldParam[i] == ',') {
                fieldParam[i] = ' ';
            }
        }
    }

    if (noSpaceComma) {
        std::string val;
        for (int i = 2; i < fieldParam.size(); i++) {
            val += fieldParam[i];
        }

        fieldList.push_back(stoi(val));
        return fieldList;
    }
    else {
        std::string val;
        for (int i = 2; i < fieldParam.size(); i++) {
            if (fieldParam[i] == ' ') {
                fieldList.push_back(stoi(val));
               val = "";
            }
            else {
                val += fieldParam[i];
            }
        }
        fieldList.push_back(stoi(val));
        return fieldList;
    }

}
int main(int argc, char* argv[]) {

    // if last arg is file type use it, otherwise use stdin.
    FILE* fileptr = ((argc > 1 && containsFileExtensions(argv[argc-1])) ? fopen(argv[argc-1], "rb") : stdin);

    // make sure delimiters and fieldParam only there once, and if not there
    // use default for the delimiter (tab) and end program if no fields.
    std::vector<int> fieldList;
    char delimiter = '\t';
    for (int i = 1; i < argc; i++) {
        if (std::string(argv[i]).find("-f") == 0) {
           fieldList = getFieldList(argv[i]);
        }
        if (std::string(argv[i]).find("-d") == 0) {
            // second index might not exist
            delimiter = argv[i][2];
        }
    }

    if (fieldList.size() == 0) {
        printf("No Fields Specified!\n");
        return 1;
    }

    // we got delimiter and field list now

    // get each line from the file, parse based on delimiter,
    // count fields, append appropriate ones to total string.

    char buffer[BUFFERSIZE] = {0};
    std::string output;
    while (fgets(buffer, sizeof(buffer), fileptr)) {
        // buffer contains string and \n character.

        // get fields separated by delimiter.
        std::vector<std::string> fields;
        std::string field;
        for (int i = 0; i < BUFFERSIZE; i++) {
            if (buffer[i] == '\n') {
                if (field != "") {
                    fields.push_back(field);
                }
                break;
            }
            else if (buffer[i] == delimiter) {
                if (field != "") {
                    fields.push_back(field);
                    field = "";
                }
            }
            else {
                // if any other character, add it to the current field.
                field += buffer[i];
            }
        }

        // for each field we got in the fields vector
        // add the relevant ones to the output string
        // based on the fieldList.
        for (auto field : fieldList) {
            output += fields[field - 1];
            output += delimiter;
        }
        // before ending line, pop last delimiter
        output.pop_back();
        output += '\n';
    }

    // don't add new line to output, may mess up outputs
    // if you want to cound number of lines with cw by
    // funneling the output of this to it.
    printf("%s", output.c_str());
    // look for field -f* where star is either number or args.

    // if starts with -f, give that to the fieldList function to get fields


    return 0;
}
