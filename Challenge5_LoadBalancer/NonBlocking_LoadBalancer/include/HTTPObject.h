#include <string> 
#include <unordered_map>

class HTTPObject {
    public: 
        std::string http_string = "";
        std::string headers = "";
        bool found_content_length = false;
        int rows_read = 0;
        int header_length;
        int current_content_length;
        bool end_of_headers = false;
        std::unordered_map<std::string, std::string> headers;

        // stages: get data -> get /r/n/r/n -> get each header as input in map
        // -> read content length -> read rest of data -> possibly add timeout

        // add to string
        void parse(char buffer[], int length);
        void parse_headers(std::string headers);


        // using the http string, start from the previous endpoint if
        // the data was received across multiple calls to read. If the
        // data is half way through a row, ignore that row for now.

        // 1. read data
        // 2. Read until you encounter \r\n\r\n
        // 3. In that case, this is the separation
        //    between headers and body
        // 4. Parse headers before this delimiter
        // 5. Get content-length, then parse body
        //    using that content length to receive
        //    as much data as there is in the body.
        // 6. Test it wil curl with long body and post req.
        // 7. This method doesn't grab every detail and might
        // 8. miss malformed headers or http.
        // 9. Can have early checks to determine if the status
        //    line is found.



};