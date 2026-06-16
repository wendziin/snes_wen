#ifndef _HTTPSTREAM_H_
#define _HTTPSTREAM_H_

#ifdef USE_CURL

#include "snes9x.h"
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <curl/curl.h>
#include "stream.h"

class HttpStream : public Stream
{
public:
    HttpStream(const char *url);
    virtual ~HttpStream(void);

    virtual int get_char(void);
    virtual char *gets(char *, size_t);
    virtual size_t read(void *, size_t);
    virtual size_t write(void *, size_t);
    virtual size_t pos(void);
    virtual size_t size(void);
    virtual int revert(uint8 origin, int32 offset);
    virtual void closeStream();

private:
    std::string url;
    size_t file_size;
    size_t current_pos;
    uint8 *cache_buffer;
    std::vector<bool> block_mask;
    
    static const size_t BLOCK_SIZE = 65536; // 64KB blocks
    
    std::thread prefetch_thread;
    std::mutex mtx;
    bool stop_thread;
    
    void prefetch_loop();
    bool fetch_range(size_t start, size_t end, uint8 *target);
    static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp);
    
    bool is_block_downloaded(size_t block_idx);
    void download_block_urgently(size_t block_idx);
};

#endif // USE_CURL

#endif
