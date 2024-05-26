#include <iostream>

#include "headers/Object.h"

struct scopeSettings {
    bool vals;
    bool key_and_val;
    bool prevKey;
};
int main() {
    std::vector<val> stack;
    std::vector<scopeSettings> scopeStack;
    TYPE valType = _NULL;
    scopeSettings settings = {false, false, false};
    std::string currentString = "";
    std::string JSON_STRING9 = R"(
{
  "key": "value",
  "key-n": 101,
  "key-o": {
    "inner key": "inner value"
  },
  "key-l": ['list value']
}
)";

    std::string JSON_STRING1 = R"(
{
    "projects": [
        {
            "name": "Project Alpha",
            "status": "completed",
            "tasks": [
                {
                    "task_id": 1,
                    "description": "Design database schema",
                    "completed": true
                },
                {
                    "task_id": 2,
                    "description": "Develop API endpoints",
                    "completed": true
                }
            ]
        },
        {
            "name": "Project Beta",
            "status": "ongoing",
            "tasks": [
                {
                    "task_id": 3,
                    "description": "Setup CI/CD pipeline",
                    "completed": false
                },
                {
                    "task_id": 4,
                    "description": "Write unit tests",
                    "completed": false
                },
                {
                    "task_id": 5,
                    "description": "Frontend development",
                    "completed": false
                }
            ]
        }
    ],
    "statistics": {
        "totalProjects": 2,
        "completedTasks": 2,
        "pendingTasks": 3
    }
}
)";



    std::string JSON_STRING11 =  R"(
    {
        "name": "John Doe",
        "age": 30,
        "is_student": false,
        "courses": ["Math", "Science", "History"]
    }
    )";

    std::string JSON_STRING7 = R"(
{
    "user": {
        "id": 12345,
        "name": "Jane Doe",
        "contact": {
            "email": "jane.doe@example.com",
            "phone": "+1234567890"
        },
        "address": {
            "street": "123 Main St",
            "city": "Anytown",
            "state": "Anystate",
            "zip": "12345"
        }
    }
}
)";

    std::string JSON_STRING = R"(
{
  "key": "value",
  "key-n": 101,
  "key-o": {},
  "key-l": []
}

    )";


    for (int i = 0; i < JSON_STRING.size(); i++) {
        char ch = JSON_STRING[i];
        if (std::isspace(static_cast<unsigned char>(ch))) {
            continue;
        }

        else if (ch == '{') {
            // push old settings on stack, use new settings
            scopeStack.push_back(settings);
            settings.key_and_val = true;
            settings.vals = false;
            settings.prevKey = false;
            stack.push_back({nullptr, OPEN_CURLY});
            continue;
        }
        else if (ch == '[') {
            scopeStack.push_back(settings);
            settings.vals = true;
            settings.key_and_val = false;
            settings.prevKey = false;
            stack.push_back({nullptr, OPEN_SQUARE});
            continue;
        }

        else if (ch == '}') {
            if (settings.prevKey) {
                printf("No corresponding value matching key!\n");
                return 1;
            }

            int startingIndex = NULL;
            for (int j = stack.size()-1; j >= 0; j--) {
                if (stack[j].type == OPEN_CURLY) {
                    startingIndex = j + 1;
                    // printf("Start index %d and end %d\n", startingIndex, stack.size());
                    // if ((stack.size() - startingIndex) % 2) {
                    //     printf("Must have equal number of keys/val\n");
                    //     return 2;
                    // }
                    break;
                }
                if (stack[j].type == OPEN_SQUARE) {
                    printf("Wrong brace, mismatch\n");
                }


            }

            Object json_obj;
            for (int j = startingIndex; j < stack.size(); j+=2) {

                // printf("Key: %s\n", ((std::string*)stack[j].data)->c_str());
                // printf("Val and type %d: ", stack[j+1].type);
                // stack[j+1].print();
                // printf("Stack size: %d curr j = %d\n", stack.size(), j);
                KeyVal keyval = {*((std::string*)stack[j].data), stack[j+1]};
                json_obj.addEntry(keyval);
            }

            for (int j = stack.size() - 1; j > startingIndex - 2; j--) {
                stack.pop_back();
            }

            val obj_val = {new Object(json_obj), OBJECT};
            stack.push_back(obj_val);

            for (int j = stack.size() - 1; j >= 0; j--) {
                if (stack[j].type == OPEN_CURLY) {
                    settings.key_and_val = true;
                    settings.vals = false;
                }
                else if (stack[j].type == OPEN_SQUARE) {
                    settings.vals = true;
                    settings.key_and_val = false;
                }
            }

            // use the settings from the top of the stack and pop it
            settings = scopeStack[scopeStack.size()-1];
            scopeStack.pop_back();

            // SET PREV KEY TO FALSE? Why? because when we close a brace whether it is
            // curly or square, it is always a value not a key, so if it is a value and it
            // is the last thing processed, then there is no prevKey, it is the prev.
            settings.prevKey = false;

            continue;
        }



        else if (ch == ']') {
            int startingIndex = NULL;
            for (int j = stack.size()-1; j >= 0; j--) {
                if (stack[j].type == OPEN_SQUARE) {
                    startingIndex = j + 1;
                    break;
                }
                if (stack[j].type == OPEN_CURLY) {
                    printf("Wrong brace, mismatch\n");
                    return 6;
                }
            }

            std::vector<val> arr;
            for (int j = startingIndex; j < stack.size(); j++) {
                // printf("Stack[%d] typpe = %d\n", j, stack[j].type);
                arr.push_back(stack[j]);
            }

            for (int j = stack.size() - 1; j > startingIndex - 2; j--) {
                stack.pop_back();
            }

            val arr_val = {new std::vector<val>(arr), ARRAY};
            stack.push_back(arr_val);

            for (int j = stack.size() - 1; j >= 0; j--) {
                if (stack[j].type == OPEN_CURLY) {
                    settings.key_and_val = true;
                    settings.vals = false;
                    break;
                }
                else if (stack[j].type == OPEN_SQUARE) {
                    settings.vals = true;
                    settings.key_and_val = false;
                    break;
                }
            }
            // NOT SURE IF THIS LINE MAKES SENSE STILL
            // IF I AM GOING TO POP THE STACK AND USE PREVIOUS
            // SETTINGS SHOULD I KEEP THIS ONE? EITHER DISCARD
            //  THIS SETTING OR MODFIY SETTINGS AFTER POPPING.
            settings = scopeStack[scopeStack.size()-1];
            scopeStack.pop_back();
            settings.prevKey = false;
            continue;
        }

        else if (ch == ':') {
            if (!settings.key_and_val || !settings.prevKey) {
                printf("Object must have keys and vals\n");
                for (int j = 0; j < stack.size(); j++)
                    stack[j].print();
                return 3;
            }
            continue;
        }

        else if (ch >= '0' && ch <= '9') {
            valType = NUMBER;
            if (settings.key_and_val && !settings.prevKey) {
                printf("Cannot have value before key\n");
                return 5;
            }
            if (settings.key_and_val)
                settings.prevKey = false;

            while (JSON_STRING[i] >= '0' && JSON_STRING[i] <= '9'){
               currentString += JSON_STRING[i];
                i++;
            }

            int number = std::stoi(currentString);
            void* data = new int(number);
            val value = {data, valType};
            stack.push_back(value);
            currentString = "";
        }

        else if (ch == '"') {
            valType = STRING;
            if (settings.key_and_val) {
                settings.prevKey = !settings.prevKey;
            }

            while (true) {
                i++;
                if (JSON_STRING[i] == '"') {
                    break;
                }
                currentString += JSON_STRING[i];
            }

            void* data = new std::string(currentString);
            stack.push_back({data, valType});
            currentString = "";
        }

        else if (ch == '\'') {
            printf("Error: Single Quotes not allowed for key or valuee.\n");
            return 7;
        }

        else if (std::isalpha(static_cast<unsigned char>(ch))) {
            if (settings.key_and_val && !settings.prevKey) {
                printf("Cannot have value before key\n");
                return 5;
            }
            if (settings.key_and_val)
                settings.prevKey = false;

            while (std::isalpha(static_cast<unsigned char>(JSON_STRING[i]))) {
                currentString += JSON_STRING[i];
                i++;
            }

            if (currentString == "true" || currentString == "false") {
                valType = BOOLEAN;
                bool booleanVal = false;
                if (currentString == "true") {
                    booleanVal = true;
                }
                val boolVal = {new bool(booleanVal), valType};
                stack.push_back(boolVal);
            }
            else if (currentString == "NULL") {
                valType = _NULL;
                stack.push_back({ nullptr, valType });
            }
            currentString = "";
        }
    }


    // printf("Stack length %d\n", stack.size());

    // printf("%d\n", stack[0].type);
    stack[0].print();

    // test to see if i can access some elements.
    // printf("]\n\n\n\n");
    // val projects = (*(Object*)stack[0].data)["projects"];
    // std::vector<val> projArray = *((std::vector<val>*)projects.data);
    // (*((Object*)projArray[0].data)).print();


    // .data["tasks"]).print();
    // Object JSON_Inner_Object, JSON_Outer_Object;
    //
    // std::vector<val> values;
    //
    // int x = 1, y = 2, z = 3;
    // val val1 = {&x, NUMBER};
    // val val2 = {&y, NUMBER};
    // val val3 = {&z, NUMBER};
    // std::string sval4 = "ABC";
    // std::string sval5 = "DEF";
    // std::string sval6 = "XYZ";
    //
    // val val4 = {(void*)sval4.c_str(), STRING };
    // val val5 = {(void*)sval4.c_str(), STRING };
    // val val6 = {(void*)sval4.c_str(), STRING };
    //
    // JSON_Inner_Object.addEntry({"Inner_num1", val1});
    // JSON_Inner_Object.addEntry({"Inner_num2", val2});
    // JSON_Inner_Object.addEntry({"Inner_str1", val4});
    //
    // val val7 = {&JSON_Inner_Object, OBJECT};
    // JSON_Outer_Object.addEntry({"num3", val3});
    // JSON_Outer_Object.addEntry({"INNER OBJ", val7});
    // JSON_Outer_Object.addEntry({"str2", val5});
    // JSON_Outer_Object.addEntry({"str3", val6});
    //
    //
    // std::vector<val> arr;
    // arr.push_back(val4);
    // arr.push_back(val5);
    // arr.push_back(val6);
    // arr.push_back(val7);
    // val arrVal = {&arr, ARRAY};
    // JSON_Outer_Object.addEntry({"OUTER ARRAY", arrVal});
    //
    //
    // JSON_Outer_Object.print();
    //
    // printf("\n\n");
    //
    // *((int*)(JSON_Outer_Object[std::string("num3")].data)) = 7;
    //
    // printf("TEST TEST\n");
    // JSON_Outer_Object["num3"].print();
    //
    // printf("\n\n\n");
    // JSON_Outer_Object.print();

    return 0;
}
