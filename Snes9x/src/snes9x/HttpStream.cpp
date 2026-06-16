#include "HttpStream.h"

#ifdef USE_CURL

#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <algorithm>

HttpStream::HttpStream(const char *u) : url(u), file_size(0), current_pos(0), cache_buffer(NULL), stop_thread(false)
{
    curl_global_init(CURL_GLOBAL_DEFAULT);
    CURL *curl = curl_easy_init();
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        if (curl_easy_perform(curl) == CURLE_OK)
        {
            double cl;
            curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &cl);
            if (cl > 0)
                file_size = (size_t)cl;
        }
        curl_easy_cleanup(curl);
    }

    if (file_size > 0)
    {
        cache_buffer = (uint8 *)calloc(1, file_size);
        block_mask.resize((file_size + BLOCK_SIZE - 1) / BLOCK_SIZE, false);
        prefetch_thread = std::thread(&HttpStream::prefetch_loop, this);
    }
}

HttpStream::~HttpStream(void)
{
    closeStream();
    if (prefetch_thread.joinable())
        prefetch_thread.join();
    if (cache_buffer)
        free(cache_buffer);
    curl_global_cleanup();
}

void HttpStream::closeStream()
{
    stop_thread = true;
}

int HttpStream::get_char(void)
{
    uint8 b;
    if (read(&b, 1) == 1)
        return b;
    return EOF;
}

char *HttpStream::gets(char *s, size_t size)
{
    size_t i = 0;
    while (i < size - 1)
    {
        int c = get_char();
        if (c == EOF)
        {
            if (i == 0) return NULL;
            break;
        }
        s[i++] = (char)c;
        if (c == '\n') break;
    }
    s[i] = '\0';
    return s;
}

size_t HttpStream::read(void *ptr, size_t size)
{
    if (current_pos >= file_size) return 0;
    size_t to_read = std::min(size, file_size - current_pos);
    
    size_t start_block = current_pos / BLOCK_SIZE;
    size_t end_block = (current_pos + to_read - 1) / BLOCK_SIZE;
    
    for (size_t i = start_block; i <= end_block; ++i)
    {
        if (!is_block_downloaded(i))
        {
            download_block_urgently(i);
        }
    }
    
    memcpy(ptr, cache_buffer + current_pos, to_read);
    current_pos += to_read;
    return to_read;
}

size_t HttpStream::write(void *, size_t)
{
    return 0; // Read-only
}

size_t HttpStream::pos(void)
{
    return current_pos;
}

size_t HttpStream::size(void)
{
    return file_size;
}

int HttpStream::revert(uint8 origin, int32 offset)
{
    size_t new_pos = pos_from_origin_offset(origin, offset);
    if (new_pos > file_size) return -1;
    current_pos = new_pos;
    return 0;
}

bool HttpStream::is_block_downloaded(size_t block_idx)
{
    std::lock_guard<std::mutex> lock(mtx);
    return block_mask[block_idx];
}

void HttpStream::download_block_urgently(size_t block_idx)
{
    size_t start = block_idx * BLOCK_SIZE;
    size_t end = std::min((block_idx + 1) * BLOCK_SIZE, file_size) - 1;
    if (fetch_range(start, end, cache_buffer + start))
    {
        std::lock_guard<std::mutex> lock(mtx);
        block_mask[block_idx] = true;
    }
}

void HttpStream::prefetch_loop()
{
    for (size_t i = 0; i < block_mask.size(); ++i)
    {
        if (stop_thread) break;
        if (!is_block_downloaded(i))
        {
            size_t start = i * BLOCK_SIZE;
            size_t end = std::min((i + 1) * BLOCK_SIZE, file_size) - 1;
            if (fetch_range(start, end, cache_buffer + start))
            {
                std::lock_guard<std::mutex> lock(mtx);
                block_mask[i] = true;
            }
        }
    }
}

size_t HttpStream::WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    uint8 *target = (uint8 *)userp;
    memcpy(target, contents, realsize);
    return realsize;
}

bool HttpStream::fetch_range(size_t start, size_t end, uint8 *target)
{
    CURL *curl = curl_easy_init();
    bool success = false;
    if (curl)
    {
        char range[64];
        snprintf(range, sizeof(range), "%zu-%zu", start, end);
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_RANGE, range);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, target);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        if (curl_easy_perform(curl) == CURLE_OK)
        {
            success = true;
        }
        curl_easy_cleanup(curl);
    }
    return success;
}
#endif // USE_CURL
