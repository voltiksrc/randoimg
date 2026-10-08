#include <string>
#include <curl/curl.h>
#include <fstream>
#include <iostream>
#include <filesystem>

using std::string;

size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata) {
  size_t total = size * nmemb;

  std::ofstream *file = static_cast<std::ofstream *>(userdata);

  file->write(ptr, total);

  return total;
}

int main() {
   string url = "https://picsum.photos/800/600"; // this url holds random images we can snatch

   string filename = "randoimg";
   std::ofstream file(filename, std::ios::binary); // open a file to write data to.
      
   // initalize libcurl
   CURL *curl = curl_easy_init();
   
   if (!curl) { 
     std::cerr<<"Failed to initalize curl.\n";
     file.close();
     return 1;
   }
   curl_easy_setopt(curl, CURLOPT_URL, url.c_str()); // make the curl url point to a random image.
   curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback); // this tells curl to use our write callback function which writes data
   curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L); // This makes curl follow redirects
   curl_easy_setopt(curl, CURLOPT_WRITEDATA, &file); // finally this tells curl where to write the data to.

   // perform the actual download here and if it fails then error, close the file
   // so no more data gets recieved, then finally quit with a error response.
   CURLcode result = curl_easy_perform(curl);
   if (result != CURLE_OK) {
     std::cerr<<curl_easy_strerror(result)<<'\n';
     file.close();
     return 1;
   }
   std::cout<<"Downloaded random image.\n";
   char* content_type = nullptr; // create a empty variable to hold the content type, example: png, jpeg, svg.
   CURLcode ctype = curl_easy_getinfo(curl, CURLINFO_CONTENT_TYPE, &content_type); // We fetch the image type to properly rename the file.

   // if it fails to fetch the type and it equals null(which is empty),
   // then print a error.
   if (ctype != CURLE_OK || content_type == nullptr) {
      std::cerr<<"Failed to get content type.\n";
   }
   string type = content_type;
   string extension;

   // If statements to check the type of file it downloaded.
   if (type == "image/jpeg") {
      extension = ".jpg";
   } else if (type == "image/png") {
      extension = ".png";
   } else if (type == "image/webp") { 
      extension = ".webp";
   } else {
      std::cerr<<"Unsupported image type.\n";
      return 1;
   }
   if (extension.empty()) {
     std::cerr<<"Unknown image format.\n"; 
     return 1;
   }
   int number = 1;
   string new_filename;
   do {
       new_filename = "randoimg_" + std::to_string(number) + extension; // checks if a randoimg already exists so it can add
       ++number;                                                        // a number to count/seperate them.
   } while (std::filesystem::exists(new_filename));
   file.close(); // close the file before renaming.
   std::filesystem::rename(filename, new_filename); // rename the file properly to match the type of image.
   std::cout<<"Saved as: "<<new_filename<<'\n';
   // free resources that were allocated for libcurl.
   curl_easy_cleanup(curl);
}
