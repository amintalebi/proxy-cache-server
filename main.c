#include "httpd.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <curl/curl.h>


int main(int c, char** v)
{
    serve_forever("8000");
    return 0;
}

void route(char* uri, char* method)
{
    if (strcmp("/", uri) == 0 && strcmp("GET", method) == 0) {
        printf("HTTP/1.1 200 OK\r\n\r\n");
        printf("Hello! You are using %s\n", request_header("User-Agent"));
    }
    else if (strcmp("/",uri)==0 && strcmp("POST", method)==0) {
        printf("HTTP/1.1 200 OK\r\n\r\n");
        printf("Wow, seems that you POSTed %d bytes. \r\n", payload_size);
        printf("Fetch the data using `payload` variable.\n");
    }
    else {
        redirect(uri);
    }
}


struct MemoryStruct {
  char *memory;
  size_t size;
};
 
static size_t
WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userp;
 
  char *ptr = realloc(mem->memory, mem->size + realsize + 1);
  if(ptr == NULL) {
    /* out of memory! */ 
    printf("not enough memory (realloc returned NULL)\n");
    return 0;
  }
 
  mem->memory = ptr;
  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;
 
  return realsize;
}
 

int redirect(char* uri) {
    CURL *curl_handle;
    CURLcode res;
    struct curl_slist *slist=NULL;
    struct MemoryStruct chunk;
    
    chunk.memory = malloc(1);  /* will be grown as needed by the realloc above */ 
    chunk.size = 0;    /* no data at this point */ 
    
    curl_global_init(CURL_GLOBAL_ALL);
    
    /* init the curl session */ 
    curl_handle = curl_easy_init();
    
    slist = curl_slist_append(slist, "Content-Type: application/json");
    slist = curl_slist_append(slist, "Content-Type: application/pdf");
    slist = curl_slist_append(slist, "Content-Type: application/javascript");
    slist = curl_slist_append(slist, "Content-Type: text/css");
    slist = curl_slist_append(slist, "Content-Type: image/png");
    slist = curl_slist_append(slist, "Content-Type: image/jpeg");
    slist = curl_slist_append(slist, "Content-Type: text/html");
    slist = curl_slist_append(slist, "Content-Type: text/plain");

    curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, slist);

    /* specify URL to get */ 
    curl_easy_setopt(curl_handle, CURLOPT_URL, uri);
    
    /* send all data to this function  */ 
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    
    /* we pass our 'chunk' struct to the callback function */ 
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
    
    /* some servers don't like requests that are made without a user-agent
        field, so we provide one */ 
    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");
    
    /* get it! */ 
    res = curl_easy_perform(curl_handle);
    
    /* check for errors */ 
    if(res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror(res));
        return -1;
    }
    
    printf("HTTP/1.1 200 OK\r\n\r\n");
    write(1, chunk.memory, chunk.size); 
    /* cleanup curl stuff */ 
    curl_easy_cleanup(curl_handle);
    
    free(chunk.memory);
    
    /* we're done with libcurl, so clean it up */ 
    curl_global_cleanup();
    
    return 0;
}
