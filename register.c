#include <stdio.h>
#include <stdlib.h>
#include <net/if.h>
#include <byteswap.h>

#include "dns_sd.h"


void customRegisterReply( DNSServiceRef sdRef, DNSServiceFlags flags, DNSServiceErrorType errorCode,
			    const char *name, const char *regtype, const char *domain, void *context)
{
    printf("In RegisterReply callback!!\n");
    printf("Error Code: %d\n", errorCode);
    printf("Service Name: %s\n", name);
    printf("RegType: %s\n", regtype);
    printf("Domain: %s\n", domain);

}

int endianChange(int i)
{
    return ((i>>8)&0xff)+((i<<8)&0xff00);
}

int main(int argc, char* argv[])
{
    DNSServiceRef ref;
    DNSServiceErrorType result;
    DNSServiceFlags nativeFlag=0;

    uint16_t port = 11234;
    char* serviceType="_personal._tcp";

    char* netInterface="eth0";
    int netIndex=if_nametoindex(netInterface);
    int serviceFD;

    char input;

    printf("Starting to register...\n");


    port = endianChange(port);	// change the endian to fit network byte order.
    result=DNSServiceRegister(
			&ref, 
			nativeFlag,
			netIndex,
			NULL,	/* service name, NULL will use computer name */
			serviceType,
			NULL,	/* domain */
			NULL,	/* host name, NULL will use computer name */
			port,	/* this number must fit network byte order */
			0,	/* txt length */
			NULL,	/* txt record */
			&customRegisterReply,	/* callback */
			NULL	/* context, user-defined that will be an argument of callback */
			);

    if (result == kDNSServiceErr_NoError){
	printf("Register service success!!\n");	
    }else{
	printf("Register service fail!!\n");
	printf("Error code: %d\n", result);
	return 1;
    }
    //serviceFD=DNSServiceRefSockFD(ref);
    // this method will active the callback register before.
    result=DNSServiceProcessResult( ref );


    if (result == kDNSServiceErr_NoError){
	printf("Register response success!!\n");	
    }else{
	printf("Register response fail!!\n");
	printf("Error code: %d\n", result);
    	return 1;
    }

    printf("press 'q' to exist:\n");
    while ((input=getchar()) != -1 && input!='q');

    DNSServiceRefDeallocate(ref);
    
    return 0;

}
