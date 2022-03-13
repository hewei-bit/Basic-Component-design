/**
 * @File Name: sync_dns_client_hw.c
 * @Brief : 同步请求池(目前只进行了dns请求)
 * @Author : hewei (hewei_1996@qq.com)
 * @Version : 1.0
 * @Creat Date : 2022-03-11
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <errno.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <sys/epoll.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <pthread.h>

#define DNS_SVR "114.114.114.114"

#define DNS_HOST 0x01
#define DNS_CNAME 0x05

// 请求与响应的格式是一致的，
// 其整体分为Header、Question、Answer、Authority、Additional 5部分，

// Header部分是一定有的，长度固定为12个字节；
// 其余4部分可能有也可能没有，并且长度也不一定，这个在Header部分中有指明。
// 1.ID：占16位。该值由发出DNS请求的程序生成，DNS服务器在响应时会使用该ID，这样便于请求程序区分不同的DNS响应。
// QR：占1位。指示该消息是请求还是响应。0表示请求；1表示响应。
// OPCODE：占4位。指示请求的类型，有请求发起者设定，响应消息中复用该值。0表示标准查询；1表示反转查询；2表示服务器状态查询。3~15目前保留，以备将来使用。
// AA（Authoritative Answer，权威应答）：占1位。表示响应的服务器是否是权威DNS服务器。只在响应消息中有效。
// TC（TrunCation，截断）：占1位。指示消息是否因为传输大小限制而被截断。
// RD（Recursion Desired，期望递归）：占1位。该值在请求消息中被设置，响应消息复用该值。如果被设置，表示希望服务器递归查询。但服务器不一定支持递归查询。
// RA（Recursion Available，递归可用性）：占1位。该值在响应消息中被设置或被清除，以表明服务器是否支持递归查询。
// Z：占3位。保留备用。
// RCODE（Response code）：占4位。该值在响应消息中被设置。取值及含义如下：
// 0：No error condition，没有错误条件；
// 1：Format error，请求格式有误，服务器无法解析请求；
// 2：Server failure，服务器出错。
// 3：Name Error，只在权威DNS服务器的响应中有意义，表示请求中的域名不存在。
// 4：Not Implemented，服务器不支持该请求类型。
// 5：Refused，服务器拒绝执行请求操作。
// 6~15：保留备用。
// QDCOUNT：占16位（无符号）。指明Question部分的包含的实体数量。
// ANCOUNT：占16位（无符号）。指明Answer部分的包含的RR（Resource Record）数量。
// NSCOUNT：占16位（无符号）。指明Authority部分的包含的RR（Resource Record）数量。
// ARCOUNT：占16位（无符号）。指明Additional部分的包含的RR（Resource Record）数量。
struct dns_header
{
    unsigned short id;
    unsigned short flags;
    unsigned short qdcount;
    unsigned short ancount;
    unsigned short nscount;
    unsigned short arcount;
};

// QNAME：字节数不定，以0x00作为结束符。表示查询的主机名。注意：众所周知，主机名被"."号分割成了多段标签。在QNAME中，每段标签前面加一个数字，表示接下来标签的长度。比如：api.sina.com.cn表示成QNAME时，会在"api"前面加上一个字节0x03，"sina"前面加上一个字节0x04，"com"前面加上一个字节0x03，而"cn"前面加上一个字节0x02；
// QTYPE：占2个字节。表示RR类型，见以上RR介绍；
// QCLASS：占2个字节。表示RR分类，见以上RR介绍。
struct dns_question
{
    int length;
    unsigned short qtype;
    unsigned short qclass;
    char *qname;
};

struct dns_item
{
    char *domain;
    char *ip;
};

int dns_create_header(struct dns_header *header)
{
    return 0;
}

int dns_create_question(struct dns_question *question, const char *hostname)
{
    return 0;
}

int dns_build_request(struct dns_header *header, struct dns_question *question, char *request)
{
    return 0;
}

static int is_pointer(int in)
{
}

static void dns_parse_name(unsigned char *chunk, unsigned char *ptr, char *out, int *len)
{
}

static int dns_parse_response(char *buffer, struct dns_item **domains)
{
    int cnt = 0;
    return cnt;
}

int dns_client_commit(const char *domain)
{
    int socketfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socketfd < 0)
    {
        perror("create socket failed \n");
        exit(1);
    }

    printf("url: %s \n", domain);

    struct sockaddr_in dest;
    bzero(&dest, sizeof(dest));
    dest.sin_family = AF_INET;
    dest.sin_port = htons(53);
    dest.sin_addr.s_addr = inet_addr(DNS_SVR);
}

char *domain[] = {
    //	"www.ntytcp.com",
    "bojing.wang",
    "www.baidu.com",
    "tieba.baidu.com",
    "news.baidu.com",
    "zhidao.baidu.com",
    "music.baidu.com",
    "image.baidu.com",
    "v.baidu.com",
    "map.baidu.com",
    "baijiahao.baidu.com",
    "xueshu.baidu.com",
    "cloud.baidu.com",
    "www.163.com",
    "open.163.com",
    "auto.163.com",
    "gov.163.com",
    "money.163.com",
    "sports.163.com",
    "tech.163.com",
    "edu.163.com",
    "www.taobao.com",
    "q.taobao.com",
    "sf.taobao.com",
    "yun.taobao.com",
    "baoxian.taobao.com",
    "www.tmall.com",
    "suning.tmall.com",
    "www.tencent.com",
    "www.qq.com",
    "www.aliyun.com",
    "www.ctrip.com",
    "hotels.ctrip.com",
    "hotels.ctrip.com",
    "vacations.ctrip.com",
    "flights.ctrip.com",
    "trains.ctrip.com",
    "bus.ctrip.com",
    "car.ctrip.com",
    "piao.ctrip.com",
    "tuan.ctrip.com",
    "you.ctrip.com",
    "g.ctrip.com",
    "lipin.ctrip.com",
    "ct.ctrip.com"};

int main(int argc, char *argv[])
{

    int count = sizeof(domain) / sizeof(domain[0]);
    int i = 0;

    for (i = 0; i < count; i++)
    {
        dns_client_commit(domain[i]);
    }

    getchar();
}
