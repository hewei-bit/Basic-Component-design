#include <cstdio>
#include <time.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h> 
#include <stdlib.h> 
#include <string.h> 
#include <malloc.h>
#include <sys/time.h>
#include <iostream>
#include <sstream> 
using namespace std;
// #define WRITE_LINE_NUM  1000000L
#define WRITE_LINE_NUM  100000L
#define BUF_LINE_NUM    1000L
#define FLUSH_LINE_INTERVAL  1000L


static int64_t get_millisecond()
{
    struct timeval tval;
    int64_t ret_tick;
    
    gettimeofday(&tval, NULL);
    
    ret_tick = tval.tv_sec * 1000L + tval.tv_usec / 1000L;
    return ret_tick;
}
// fwrite 1000000 line, fwrite:10, fflush:1, buf_size:2900000, need time:409ms, ops:2444987
// write  1000000 line,  write:10,  fsync:1, buf_size:2900000, need time:408ms, ops:2450980
// fwrite 1000000 line, fwrite:100, fflush:1, buf_size:290000, need time:437ms, ops:2288329
// write  1000000 line,  write:100,  fsync:1, buf_size:290000, need time:419ms, ops:2386634
// fwrite 1000000 line, fwrite:10000, fflush:1, buf_size:2900, need time:3128ms, ops:319693
// write  1000000 line,  write:10000,  fsync:1, buf_size:2900, need time:1784ms, ops:560538
// fwrite 1000000 line, fwrite:20000, fflush:1, buf_size:1450, need time:4198ms, ops:238208
// write  1000000 line,  write:20000,  fsync:1, buf_size:1450, need time:3043ms, ops:328623
// fwrite 1000000 line, fwrite:28571, fflush:1, buf_size:1015, need time:4213ms, ops:237360
// write  1000000 line,  write:28571,  fsync:1, buf_size:1015, need time:4221ms, ops:236910
// fwrite 1000000 line, fwrite:50000, fflush:1, buf_size:580, need time:4240ms, ops:235849
// write  1000000 line,  write:50000,  fsync:1, buf_size:580, need time:7005ms, ops:142755

// fwrite 1000000 line, fwrite:1000000, fflush:100, need time:4206ms, ops:237755
// write  1000000 line,  write:1000000,  fsync:100, need time:135555ms, ops:7377
// fwrite 1000000 line, fwrite:100000, fflush:100, need time:4240ms, ops:235849
// write  1000000 line,  write:100000,  fsync:100, need time:13798ms, ops:72474
// fwrite 1000000 line, fwrite:10000, fflush:100, need time:3100ms, ops:322580
// write  1000000 line,  write:10000,  fsync:100, need time:1728ms, ops:578703



void fwrite_test(int64_t write_line_num, int64_t buf_line_num, int64_t flush_interval) 
{
    FILE *file = fopen("1-fwrite_test.log", "wt");
    string line_string;
    string buf_string;
    int buf_count = 0;
    int flush_count = 0;
    int buf_size = 0;
    int64_t begin_time = get_millisecond();
    for(int i = 0; i < write_line_num; i++) {
        ostringstream oss;
        oss<<"NO." << i<<" Root Error Message!";        // 47字节 * 10000 = 470000=470k
        buf_string.append(oss.str());

        if(++buf_count >= buf_line_num)   {
            buf_count = 0;
            buf_size = buf_string.size();
            fwrite(buf_string.c_str(), 1, buf_string.size(),  file);
            buf_string.clear();
        }
        if(++flush_count >= flush_interval)   {
            flush_count = 0;
            // fflush(file);
        }
    }

    fclose(file);
    int64_t end_time = get_millisecond();
    int64_t need_time = end_time - begin_time;
    if(need_time < 1) {
        printf("%s 写入时间太短, 请增加写入的行数 \n", __FUNCTION__);
        return;
    }
    int64_t ops = write_line_num * 1000/need_time;
    printf("fwrite %ld line, fwrite:%ld, fflush:%ld, buf_size:%d, need time:%ldms, ops:%ld\n",
        write_line_num, write_line_num/buf_line_num, write_line_num/flush_interval,buf_size, need_time, ops);
}

 
void write_test(int64_t write_line_num, int64_t buf_line_num, int64_t flush_interval) 
{
    off_t file = open("1-write_test.log",O_RDWR |O_CREAT | O_TRUNC,  S_IRUSR | S_IWUSR);
    if(file < 0) {
        printf("%s open fialed\n", __FUNCTION__);
        return;
    }
    int64_t begin_time = get_millisecond();
    string line_string;
    string buf_string;
    int buf_count = 0;
    int flush_count = 0;
    int buf_size = 0;
    for(int i = 0; i < write_line_num; i++) {
        ostringstream oss;
        oss<<"NO." << i<<" Root Error Message!";        // 47字节 * 10000 = 470000=470k
        buf_string.append(oss.str());

        if(++buf_count >= buf_line_num)   {
            buf_count = 0;
            buf_size = buf_string.size();
            write(file,buf_string.c_str(), buf_string.size());
            buf_string.clear();
        }
        if(++flush_count >= flush_interval)   {
            flush_count = 0;
            fsync(file);
        }
    }

    close(file);
    int64_t end_time = get_millisecond();
    int64_t need_time = end_time - begin_time;
    if(need_time < 1) {
        printf("%s 写入时间太短, 请增加写入的行数 \n", __FUNCTION__);
        return;
    }
    int64_t ops = write_line_num * 1000/need_time;
    printf("write  %ld line,  write:%ld,  fsync:%ld, buf_size:%d, need time:%ldms, ops:%ld\n",
        write_line_num, write_line_num/buf_line_num, write_line_num/flush_interval,buf_size, need_time, ops);
}

// g++ -g -o 1-file_test 1-file_test.cpp
int main(int argc, char** argv)
{
    if(argc !=1 && argc!=4) {
        printf("usage 1-file_test write_num buf_num flush_interval\n");
        exit(1);
    }
    int64_t write_line_num = WRITE_LINE_NUM;
    int64_t buf_line_num = BUF_LINE_NUM;
    int64_t flush_interval =FLUSH_LINE_INTERVAL;

    if(argc == 4) {
        write_line_num = strtol(argv[1], NULL, 10);
        buf_line_num = strtol(argv[2], NULL, 10);
        flush_interval = strtol(argv[3], NULL, 10);
    }

    // printf("main   %ld line,  write:%ld,  fsync:%ld\n", write_line_num, write_line_num/buf_line_num, write_line_num/flush_interval);

    for(int i = 0; i< 5; i++) {
        fwrite_test(write_line_num,buf_line_num, flush_interval);
        write_test(write_line_num,buf_line_num, flush_interval);
        printf("\n");
    }
}